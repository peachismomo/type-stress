#include <catch2/catch_all.hpp>
#include <stress/stress_reflection.hpp>

STRESS_STRUCT(ExampleStruct)
{
    int a;
    float b;
    bool c;

    STRESS_FIELDS(
        STRESS_FIELD(a),
        STRESS_FIELD(b),
        STRESS_FIELD(c))
};

TEST_CASE("Foo reflection")
{
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties.size() == 3);

    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[0].offset == 0);
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[1].offset == 4);
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[2].offset == 8);

    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[0].name == "a");
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[1].name == "b");
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[2].name == "c");

    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[0].size == sizeof(int));
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[1].size == sizeof(float));
    REQUIRE(stress::getTypeInfo<ExampleStruct>().properties[2].size == sizeof(bool));
}
