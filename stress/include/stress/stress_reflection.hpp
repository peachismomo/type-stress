#pragma once
#include <tuple>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string_view>
#include <cstddef>

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
    struct FieldInfo
    {
        std::string_view name;
        std::type_index type;
        size_t offset;
        size_t size;

        bool isSerializable = false;
        bool isPrivate = false;
        bool isReadonly = false;

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
