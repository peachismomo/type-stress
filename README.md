# stress_reflection.hpp (header-only)

A tiny, header-only **runtime reflection** and **enum-to-string** helper for C++20. It lets you:

* Describe a type’s fields once (name, type, offset, size, flags).
* **Auto-register** that metadata into a global registry at static init.
* Read/write fields at runtime (type-checked), with optional `onChange`.
* Detect “container-ish” types (vector/map/set/tuple/optional/variant/string/enum).
* Provide a unified `AnyToStringRuntime()` that can stringify primitives, strings, registered enums, and custom field stringify functions.

> For a complete usage walkthrough, see `example.cpp`.

---

## What this provides

### 1) Type + field metadata

* **`stress::FieldInfo`**: per-field metadata

  * `name`, `type` (`std::type_index`), `offset`, `size`
  * flags: `isSerializable`, `isPrivate`, `isReadonly`
  * `containerKind` (auto-detected)
  * optional hooks: `onChange`, `toString`

* **`stress::TypeInfo`**: per-type metadata

  * `typeName`
  * `properties` (`std::vector<FieldInfo>`)

### 2) Global registries

* `stress::gTypeRegistry[typeid(T)] -> TypeInfo`
* `stress::gToStringRegistry[typeid(EnumType)] -> ToStringFn`

These are filled automatically via the macros below.

### 3) Runtime access helpers

Inside `stress::access`:

* `getFieldPtr(obj, field)` (void* pointer arithmetic via `field.offset`)
* `setFieldValue(obj, field, value)`

  * fails if `field.isReadonly` or `typeid(Value)` mismatch
  * calls `field.onChange` if present
* `getFieldValue(obj, field, out)`
* `AnyToStringRuntime(field, ptr)`

  * handles `std::string`, registered enums, custom `FieldInfo::toString`
  * otherwise returns `"<unregistered: ...>"`

---

## Reflection workflow (how it’s meant to be used)

1. Make your type inherit from `stress::Reflectable<T>` using the macros:

   * `STRESS_STRUCT(T)` / `STRESS_CLASS(T)` (or `_INHERIT` variants)

2. Provide a `fields()` function returning a tuple of `FieldInfo`:

   * `STRESS_FIELDS(STRESS_FIELD(a), STRESS_FIELD(b), ...)`

3. The `STRESS_FIELDS(...)` macro also instantiates:

   * `static inline ::stress::AutoRegister<Self> __auto_reg{};`
     which registers the type into `gTypeRegistry` at program start.

Once registered, you can query:

* `stress::getTypeInfo<T>()` (by template type)
* `stress::getTypeInfo(typeid(T))` (by runtime `type_index`)

---

## Field macros

All `STRESS_*FIELD*` macros expand to calls that create `stress::FieldInfo` via:

* `stress::makeField(name, memberPtr, isSerializable, isPrivate, isReadonly)`
* `stress::makeFieldWithToString(...)` (custom stringify thunk)

### Common ones

* `STRESS_FIELD(member)`
  Public, not serializable, not readonly.

* `STRESS_PRIVATE_FIELD(member)`
  Marks field as “private” for tooling/UI (does *not* enforce C++ access control).

* `STRESS_READONLY_FIELD(member)`
  Blocks runtime `set(...)` calls.

* `STRESS_FIELD_SERIALIZABLE(member)`
  Same as `STRESS_FIELD` but `isSerializable = true`.

### Custom to-string

* `STRESS_FIELD_WITH_TOSTRING_BODY(member, BODY)`
  Lets you embed a tiny lambda body that returns `std::string`.
  Useful for types that don’t have `operator<<` and you want a nice display string.

---

## Enum helpers

This header includes a small “X-macro” enum pattern that gives you:

* `EnumToString(EnumType)` (a `constexpr std::string_view` function)
* `EnumTypeValues` (array of all enumerators)
* `EnumTypeCount`
* registration into `gToStringRegistry` so runtime stringify works

### Macros

* `STRESS_ENUM_DEFINE(EnumType, LIST_MACRO)`
* `STRESS_ENUM_REGISTER(EnumType)`
* `STRESS_ENUM_DEFINE_VALUES(EnumType, LIST_MACRO)`
* `STRESS_ENUM_DEFINE_AND_REGISTER(EnumType, LIST_MACRO)` (the usual one)
* `STRESS_ENUM_DECLARE_DEFINE_REGISTER(EnumType, Underlying, LIST_MACRO)` (declares enum + all helpers)

The registration uses an internal `AutoRegisterFn` to insert a `ToStringFn` into `gToStringRegistry[typeid(EnumType)]`.

---

## Container classification

`stress::ContainerKind` is computed at compile time via `getContainerKind<T>()`, based on C++20 concepts / traits:

* `StringLike` → `String`
* `EnumLike` → `Enum`
* `MapLike` → `Map`
* `SetLike` → `Set`
* `FixedArrayLike` (C array or `std::array`) → `FixedArray`
* `VectorLike` (iterable + `size()` + `operator[]`, excluding string/map/set) → `Vector`
* `TupleLike` → `Tuple`
* `OptionalLike` → `Optional`
* `VariantLike` → `Variant`
* else → `None`

This is stored into `FieldInfo::containerKind` automatically by `makeField()`.

---

## Key types and functions

### `stress::Reflectable<T>`

Base class your types inherit from. Provides:

* `static TypeInfo makeTypeInfo()`
* `static void registerType()`
* `bool set(field, value)`
* `bool get(field, out)`

### `stress::AutoRegister<T>`

Used by `STRESS_FIELDS(...)` to auto-register a reflectable type.

### `stress::makeField(...)`

Builds `FieldInfo`, computes:

* member offset (`member_offset`)
* size (`sizeof(Member)`)
* readonly flag (explicit or `const` member)
* container kind

### `stress::access::AnyToStringRuntime(...)`

Main runtime stringifier:

* checks string, then enum registry, then `FieldInfo::toString`

---

## Notes / caveats

* **Header-only, global state:** `gTypeRegistry` and `gToStringRegistry` are `inline` globals.
* **Static initialization order:** registration happens during static init; if you query registries very early (before those translation units run), you can hit missing entries.
* **Offsets:** `member_offset` uses a non-zero “fake base” pointer to avoid null warnings. This pattern assumes typical object layout rules for standard data members.
* **“Private” is a flag, not enforcement:** `isPrivate` is metadata only.
* **Type checking:** runtime `set/get` require exact `typeid` matches.

---

## Where to look next

* See `example.cpp` for:

  * declaring a reflectable struct/class
  * registering fields
  * defining + registering enums
  * iterating `TypeInfo.properties`
  * runtime set/get + runtime stringify
