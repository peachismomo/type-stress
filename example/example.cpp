#include <iostream>
#include <string>
#include <stress/stress_reflection.hpp>

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

    auto &typeInfo3 = stress::getTypeInfo<int>();
    std::cout << typeid(int).name() << " no. of properties=" << typeInfo3.properties.size() << std::endl;

    ExampleStruct exampleStruct(1, 1.f, false);
}
