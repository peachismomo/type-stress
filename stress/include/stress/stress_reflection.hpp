#pragma once
#include <tuple>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <string_view>
#include <cstddef>
#include <type_traits>
#include <concepts>
#include <tuple>
#include <optional>
#include <variant>
#include <array>

#define STRESS_FIELD(member) \
    ::stress::makeField(#member, &Self::member, false, false, false)

#define STRESS_PRIVATE_FIELD(member) \
    ::stress::makeField(#member, &Self::member, false, true, false)

#define STRESS_READONLY_FIELD(member) \
    ::stress::makeField(#member, &Self::member, false, false, true)

#define STRESS_PRIVATE_READONLY_FIELD(member) \
    ::stress::makeField(#member, &Self::member, false, true, true)

#define STRESS_FIELD_SERIALIZABLE(member) \
    ::stress::makeField(#member, &Self::member, true, false, false)

#define STRESS_PRIVATE_FIELD_SERIALIZABLE(member) \
    ::stress::makeField(#member, &Self::member, true, true, false)

#define STRESS_READONLY_FIELD_SERIALIZABLE(member) \
    ::stress::makeField(#member, &Self::member, true, false, true)

#define STRESS_PRIVATE_READONLY_FIELD_SERIALIZABLE(member) \
    ::stress::makeField(#member, &Self::member, true, true, true)

#define STRESS_DEFAULT_CTOR() \
    Self() = default;

#define STRESS_STRUCT(T) \
    struct T : public ::stress::Reflectable<T>

#define STRESS_CLASS(T) \
    class T : public ::stress::Reflectable<T>

#define STRESS_STRUCT_INHERIT(T, ...) \
    struct T : public ::stress::Reflectable<T>, __VA_ARGS__

#define STRESS_CLASS_INHERIT(T, ...) \
    class T : public ::stress::Reflectable<T>, __VA_ARGS__

#define STRESS_FIELDS(...)                                        \
    static auto fields() { return std::make_tuple(__VA_ARGS__); } \
    static inline ::stress::AutoRegister<Self> __auto_reg{};

namespace stress
{
    enum class ContainerKind : char
    {
        None,       // Not a container
        Vector,     // std::vector, deque, dynamic array
        FixedArray, // std::array, C array
        Map,        // std::map, unordered_map
        Set,        // std::set, unordered_set
        String,     // std::string
        Tuple,      // std::tuple / std::pair
        Optional,   // std::optional
        Variant     // std::variant
    };

    template <typename T>
    concept Iterable =
        requires(T t) {
            std::begin(t);
            std::end(t);
        };

    template <typename T>
    concept StringLike =
        std::same_as<std::remove_cvref_t<T>, std::string> ||
        std::same_as<std::remove_cvref_t<T>, std::string_view>;

    template <typename T>
    concept MapLike =
        Iterable<T> &&
        requires {
            typename T::key_type;
            typename T::mapped_type;
        };

    template <typename T>
    concept SetLike =
        Iterable<T> &&
        requires { typename T::key_type; } &&
        (!requires { typename T::mapped_type; });

    template <class T>
    struct is_std_array : std::false_type
    {
    };
    template <class U, size_t N>
    struct is_std_array<std::array<U, N>> : std::true_type
    {
    };

    template <class T>
    inline constexpr bool is_std_array_v = is_std_array<std::remove_cvref_t<T>>::value;

    template <class T>
    inline constexpr bool is_c_array_v = std::is_array_v<std::remove_reference_t<T>>;

    template <typename T>
    concept FixedArrayLike = is_std_array_v<T> || is_c_array_v<T>;

    template <typename T>
    concept TupleLike =
        requires {
            typename std::tuple_size<std::remove_cvref_t<T>>::type;
        };

    template <class T>
    struct is_optional : std::false_type
    {
    };
    template <class U>
    struct is_optional<std::optional<U>> : std::true_type
    {
    };

    template <class T>
    inline constexpr bool is_optional_v = is_optional<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept OptionalLike = is_optional_v<T>;

    template <class T>
    struct is_variant : std::false_type
    {
    };
    template <class... U>
    struct is_variant<std::variant<U...>> : std::true_type
    {
    };

    template <class T>
    inline constexpr bool is_variant_v = is_variant<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept VariantLike = is_variant_v<T>;

    // NOTE: exclude StringLike + MapLike + SetLike to avoid misclassification
    template <typename T>
    concept VectorLike =
        Iterable<T> &&
        (!StringLike<T>) &&
        (!MapLike<T>) &&
        (!SetLike<T>) &&
        requires(T t) {
            typename T::value_type;
            { t.size() } -> std::convertible_to<size_t>;
            { t[0] }; // supports operator[]
        };

    template <typename T>
    constexpr ContainerKind getContainerKind()
    {
        if constexpr (StringLike<T>)
            return ContainerKind::String;
        else if constexpr (MapLike<T>)
            return ContainerKind::Map;
        else if constexpr (SetLike<T>)
            return ContainerKind::Set;
        else if constexpr (FixedArrayLike<T>)
            return ContainerKind::FixedArray;
        else if constexpr (VectorLike<T>)
            return ContainerKind::Vector;
        else if constexpr (TupleLike<T>)
            return ContainerKind::Tuple;
        else if constexpr (OptionalLike<T>)
            return ContainerKind::Optional;
        else if constexpr (VariantLike<T>)
            return ContainerKind::Variant;
        else
            return ContainerKind::None;
    }
    struct FieldInfo
    {
        std::string_view name;
        std::type_index type;
        size_t offset;
        size_t size;

        bool isSerializable = false;
        bool isPrivate = false;
        bool isReadonly = false;
        ContainerKind containerKind = ContainerKind::None;

        using OnChangeFn = void (*)(void *obj, const FieldInfo &field);
        OnChangeFn onChange = nullptr;
    };

    struct TypeInfo
    {
        std::string_view typeName;
        std::vector<FieldInfo> properties;
    };

    namespace access
    {
        inline void *getFieldPtr(void *obj, const FieldInfo &field)
        {
            return reinterpret_cast<char *>(obj) + field.offset;
        }

        inline const void *getFieldPtr(const void *obj, const FieldInfo &field)
        {
            return reinterpret_cast<const char *>(obj) + field.offset;
        }

        template <typename Value>
        inline bool setFieldValue(void *obj, const FieldInfo &field, const Value &value)
        {
            void *dst = getFieldPtr(obj, field);

            if (field.isReadonly)
                return false;

            if (field.type != typeid(Value))
                return false;

            *reinterpret_cast<Value *>(dst) = value;

            if (field.onChange)
                field.onChange(obj, field);

            return true;
        }

        template <typename Value>
        inline bool getFieldValue(const void *obj, const FieldInfo &field, Value &outValue)
        {
            if (field.type != typeid(Value))
                return false;

            const void *src = getFieldPtr(obj, field);
            outValue = *reinterpret_cast<const Value *>(src);
            return true;
        }
    }

    inline std::unordered_map<std::type_index, TypeInfo> gTypeRegistry;

    template <typename T>
    struct Reflectable
    {
        Reflectable() = default;

        using Self = T;

        static TypeInfo makeTypeInfo()
        {
            TypeInfo info;
            info.typeName = typeid(T).name();

            auto tup = T::fields();
            info.properties.reserve(std::tuple_size_v<decltype(tup)>);

            std::apply([&](auto &&...f)
                       { (info.properties.push_back(f), ...); }, tup);

            return info;
        }

        static void registerType()
        {
            gTypeRegistry[typeid(T)] = makeTypeInfo();
        }

        template <typename Value>
        bool set(const FieldInfo &field, const Value &value)
        {
            return ::stress::access::setFieldValue(static_cast<T *>(this), field, value);
        }

        template <typename Value>
        bool get(const FieldInfo &field, Value &out)
        {
            return ::stress::access::getFieldValue(static_cast<const T *>(this), field, out);
        }
    };

    template <typename T>
    struct AutoRegister
    {
        AutoRegister() { Reflectable<T>::registerType(); }
    };

    template <typename Class, typename Member>
    constexpr std::size_t member_offset(Member Class::*m)
    {
        // Pick a non-zero base to avoid null-pointer-subtraction warnings
        constexpr std::uintptr_t kBase = 0x1000;
        auto base = reinterpret_cast<const Class *>(kBase);
        auto mem = &(base->*m);
        return reinterpret_cast<std::uintptr_t>(mem) - kBase;
    }

    template <typename Class, typename Member>
    FieldInfo makeField(const char *name, Member Class::*m,
                        bool isSerializable = false,
                        bool isPrivate = false,
                        bool isReadonly = false)
    {
        return FieldInfo{
            name,
            typeid(Member),
            member_offset(m),
            sizeof(Member),
            isSerializable,
            isPrivate,
            isReadonly ? true : std::is_const_v<Member>,
            getContainerKind<Member>(),
            nullptr};
    }

    template <typename T>
    const TypeInfo &getTypeInfo()
    {
        return gTypeRegistry[typeid(T)];
    }

    template <typename T>
    const FieldInfo &getFieldInfo([[maybe_unused]] const TypeInfo &typeInfo, [[maybe_unused]] std::type_index &ti)
    {
    }

    inline const TypeInfo *getTypeInfo(const std::type_index &ti)
    {
        auto it = gTypeRegistry.find(ti);
        if (it != gTypeRegistry.end())
            return &it->second;
        return nullptr; // type not reflectable
    }
} // namespace stress
