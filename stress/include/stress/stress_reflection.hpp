#pragma once
#include <tuple>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string_view>
#include <cstddef>

#define STRESS_FIELD(member) \
    ::stress::makeField(#member, &Self::member)

#define STRESS_STRUCT(T) \
    struct T : public ::stress::Reflectable<T>

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

        using OnChangeFn = void (*)(void *obj, const FieldInfo &field);
        OnChangeFn onChange = nullptr;
    };

    struct TypeInfo
    {
        std::string_view typeName;
        std::vector<FieldInfo> properties;
    };

    inline std::unordered_map<std::type_index, TypeInfo> gTypeRegistry;

    template <typename T>
    struct Reflectable
    {
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
    };

    template <typename T>
    struct AutoRegister
    {
        AutoRegister() { Reflectable<T>::registerType(); }
    };

    template <typename Class, typename Member>
    FieldInfo makeField(const char *name, Member Class::*m)
    {
        return FieldInfo{
            name,
            typeid(Member),
            size_t(reinterpret_cast<char *>(&(reinterpret_cast<Class *>(0)->*m)) - reinterpret_cast<char *>(0)),
            sizeof(Member),
            nullptr};
    }

    template <typename T>
    TypeInfo &getTypeInfo()
    {
        return gTypeRegistry[typeid(T)];
    }

} // namespace stress
