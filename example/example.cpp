#include <iostream>
#include <string>
#include <stress/stress_reflection.hpp>

#define EXAMPLE_ENUM_ITEMS(X, EnumType) \
    X(EnumType, ONE)                    \
    X(EnumType, TWO)                    \
    X(EnumType, THREE)

enum class ExampleEnum : uint8_t
{
    ONE,
    TWO,
    THREE
};

STRESS_ENUM_DEFINE_AND_REGISTER(ExampleEnum, EXAMPLE_ENUM_ITEMS)

STRESS_STRUCT(ExampleStruct)
{
    ExampleStruct() = default;

    ExampleStruct(int a_, float b_, bool c_)
        : a(a_), b(b_), c(c_) {}

    int a;
    float b;
    bool c;

    STRESS_FIELDS(
        STRESS_FIELD(a),
        STRESS_FIELD(b),
        STRESS_FIELD(c))
};

STRESS_STRUCT(ExampleStruct2)
{
    ExampleStruct d;
    std::vector<int> e;
    std::unordered_map<std::string, int> f;

    STRESS_FIELDS(
        STRESS_FIELD(d),
        STRESS_FIELD(e),
        STRESS_FIELD(f))
};

struct TempStruct
{
    int a, b;
};

STRESS_STRUCT(ExampleStruct3)
{
    ExampleEnum g;
    TempStruct h;
    std::string i;

    STRESS_FIELDS(
        STRESS_FIELD(g),
        STRESS_FIELD_WITH_TOSTRING_BODY(h, {
            return "TempStruct{a=" + std::to_string(f.a) + ", b=" + std::to_string(f.b) + "}";
        }),
        STRESS_FIELD(i))
};

// Inheritance doesn't work yet
// STRESS_STRUCT_INHERIT(ExampleStruct3, public ExampleStruct, public ExampleStruct2)
// {
//     int g, h, i;
//     STRESS_FIELDS(
//         STRESS_FIELD(g),
//         STRESS_FIELD(h),
//         STRESS_FIELD(i))
// };

int main()
{
    auto &typeInfo = stress::getTypeInfo<ExampleStruct>();
    std::cout << typeid(ExampleStruct).name() << " no. of properties=" << typeInfo.properties.size() << std::endl;

    for (auto &f : typeInfo.properties)
    {
        std::cout << f.name << " offset=" << f.offset
                  << " size=" << f.size << " type=" << f.type.name() << " iterable=" << int(f.containerKind) << "\n";
    }

    auto &typeInfo2 = stress::getTypeInfo<ExampleStruct2>();
    std::cout << typeid(ExampleStruct2).name() << " no. of properties=" << typeInfo2.properties.size() << std::endl;

    for (auto &f : typeInfo2.properties)
    {
        std::cout << f.name << " offset=" << f.offset
                  << " size=" << f.size << " type=" << f.type.name() << " iterable=" << int(f.containerKind) << "\n";
    }

    auto &typeInfo3 = stress::getTypeInfo<ExampleStruct3>();
    ExampleStruct3 exampleStruct3;
    exampleStruct3.g = ExampleEnum::ONE;
    exampleStruct3.i = "Hello";
    std::cout << typeid(int).name() << " no. of properties=" << typeInfo3.properties.size() << std::endl;
    for (auto &f : typeInfo3.properties)
    {
        auto ptr = stress::access::getFieldPtr(&exampleStruct3, f);
        std::cout << "ToString: " << stress::access::AnyToStringRuntime(f, ptr) << std::endl;
    }

    for (ExampleEnum e : ExampleEnumValues)
    {
        std::cout << EnumToString(e) << std::endl;
    }

    ExampleStruct exampleStruct(1, 1.f, false);
}
