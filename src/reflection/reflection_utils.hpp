#pragma once

/*
*   Thanks to this youtube channel for providing the base on which the
*   component reflection system is built upon.
* 
*   If you wan't to learn more about how it works, be sure to check him out!
*   https://www.youtube.com/@ConnorDeMeyer.
*/

constexpr float DEFAULT_SPEED = 0.1f;

// Registers the component to the compile time reflection system.
// It is important to put this at the top of the class before any other declarations.
#define RegisterComponent(NewClassName) \
    using ClassName = NewClassName; \
    template<size_t Line> struct CategoryResolver { static constexpr const char* Value = CategoryResolver<Line - 1>::Value; }; \
    template<> struct CategoryResolver<0> { static constexpr const char* Value = ""; }; \
    REGISTER_TYPE(NewClassName); \
    public: \
    virtual TypeId GetTypeId() const override { return TypeId::Create<NewClassName>(); } \
    private:

#define ExportPropertyCategory(NewGroupName) \
    template<> struct CategoryResolver<__LINE__> { static constexpr const char* Value = NewGroupName; };

// Marks member variable as an exported property.
// 
// This is the generic export property macro that works on any data type that is supported by the inspector.
// There are more advanced export options that provide granular control over how the property is displayed in the inspector.
// 
// Supports:
// Any Type
// 
// Usage: ExportProperty(float, value, 1.0f)
#define ExportProperty(MemberType, MemberName, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName, (ExportInfo{"", 0.0f, 0.0f, DEFAULT_SPEED, ExportFlags_None, CategoryResolver<__LINE__>::Value}));

// Exports property clamping it between min and max.
// Must be provided with a unit prefix that is displayed after the value in the inspector.
//                 
// Supports:
// int
// float
//                         
// Usage: 
// ExportPropertyRanged(<type>, <name>, std::string <unit>, float <min>, float <max>, float <speed>, <initial value>)
// 
// Example: 
// ExportPropertyRanged(float, "speed", "m/s", 0.0f, 100.0f, 0.05f, 2.0f)
#define ExportPropertyRanged(MemberType, MemberName, MemberUnit, MemberMin, MemberMax, MemberSpeed, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName, (ExportInfo{MemberUnit, MemberMin, MemberMax, MemberSpeed, ExportFlags_None, CategoryResolver<__LINE__>::Value}));

// Exports property as an angle meaning it will be displayed in the inspector in degrees
// even though the underlying unit is radians.
// 
// Supports:
// float
//                                         
// Usage: 
// ExportPropertyAngle(<type>, <name>, <initial value>)
// 
// Example: 
// ExportPropertyAngle(float, rotation, 3.14 / 2.0)
#define ExportPropertyAngle(MemberType, MemberName, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName, (ExportInfo{"degrees", 0, 0, DEFAULT_SPEED, ExportFlags_Angle, CategoryResolver<__LINE__>::Value}));

// Exports property as a clamped angle between min and max. 
// It will be displayed in the inspector in degrees even though the underlying unit is radians.
// Min and Max values should be provided in radians.
// 
// Supports:
// float
//                                         
// Usage: 
// ExportPropertyClampedAngle(<type>, <name>, float <min>, float <max>, <initial value>)
// 
// Example: 
// ExportPropertyClampedAngle(float, rotation, 0.0f, 3.14 / 4.0, 0.0f)
#define ExportPropertyClampedAngle(MemberType, MemberName, MemberMin, MemberMax, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName, (ExportInfo{"degrees", MemberMin, MemberMax, DEFAULT_SPEED, ExportFlags_Angle, CategoryResolver<__LINE__>::Value}));

// Exports property as a color making it editable via a color wheel.
// 
// Supports:
// XMFLOAT3
// XMFLOAT4
//                                         
// Usage: 
// ExportPropertyColor(<type>, <name>, <initial value>)
// 
// Example: 
// ExportPropertyColor(XMFLOAT3, color, {1.0f, 0.0f, 0.0f})
#define ExportPropertyColor(MemberType, MemberName, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName, (ExportInfo{"", 0.0f, 0.0f, DEFAULT_SPEED, ExportFlags_ColorWheel, CategoryResolver<__LINE__>::Value}));

/*
*   Easy way to get a reference to any member variable inside a class or struct
*   given it's byte offset.
*/
template <typename T, typename Type>
T &GetMemberByOffset(Type *typePtr, size_t byteOffset)
{
    return *reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(typePtr) + byteOffset);
}