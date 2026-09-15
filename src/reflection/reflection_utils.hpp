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
*   PROPERTY CHANGED CALLBACKS
*/

#define _REFLECTION_CONCAT_INNER(a, b) a##b
#define _REFLECTION_CONCAT(a, b) _REFLECTION_CONCAT_INNER(a, b)

// Binds a member function that is called whenever the given exported property is written
// to from the outside (the inspector, a scene loader, an undo step, ...).
//
// Place it on the line directly BELOW the Export* macro of the property it belongs to.
// It has to come after the property because it needs the member's byte offset, but the
// callback function itself may be declared anywhere in the class (even further down,
// and it may be private) - the body of the generated dispatcher is only parsed once the
// class is complete.
//
// The callback takes no arguments and may be const. Binding several callbacks to the
// same property is allowed; they run in registration order.
//
// Supports:
// Any exported property
//
// Usage:
// OnPropertyChanged(<name>, <member function>)
//
// Example:
// ExportProperty(XMFLOAT3, localPosition, {0.0f, 0.0f, 0.0f})
// OnPropertyChanged(localPosition, MarkLocalDirty)
#define ExportPropertyChanged(MemberName, MemberCallback) \
    static void _REFLECTION_CONCAT(ReflectionPropertyCallback_, __LINE__)(void *instance) \
    { \
        static_cast<ClassName *>(instance)->MemberCallback(); \
    } \
    inline static const RegisterPropertyCallback _REFLECTION_CONCAT(ReflectionPropertyCallbackReg_, __LINE__){ \
        TypeId::Create<ClassName>(), \
        MemberPointerHelper<ClassName, decltype(MemberName)>::GetOffset(&ClassName::MemberName), \
        &ClassName::_REFLECTION_CONCAT(ReflectionPropertyCallback_, __LINE__)}; \
    static_assert(true, "")

// Binds a member function that is called whenever ANY exported property of the class is
// written to from the outside. Runs after the property specific callbacks, if any.
// Can be placed anywhere inside the class, it does not refer to a member variable.
//
// Usage:
// OnAnyPropertyChanged(<member function>)
#define OnAnyPropertyChanged(MemberCallback) \
    static void _REFLECTION_CONCAT(ReflectionAnyPropertyCallback_, __LINE__)(void *instance) \
    { \
        static_cast<ClassName *>(instance)->MemberCallback(); \
    } \
    inline static const RegisterPropertyCallback _REFLECTION_CONCAT(ReflectionAnyPropertyCallbackReg_, __LINE__){ \
        TypeId::Create<ClassName>(), \
        MemberId::AnyMember, \
        &ClassName::_REFLECTION_CONCAT(ReflectionAnyPropertyCallback_, __LINE__)}; \
    static_assert(true, "")

/*
*   Easy way to get a reference to any member variable inside a class or struct
*   given it's byte offset.
*/
template <typename T, typename Type>
T &GetMemberByOffset(Type *typePtr, size_t byteOffset)
{
    return *reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(typePtr) + byteOffset);
}

/*
*  Convenience function for retreiving the hash for a specific type
*  at compile time. Useful when you wan't to switch behaviour depending on type.
*/
template <typename T>
consteval uint64_t GetTypeHash()
{
    VariableId variableId = VariableId::Create<T>();
    return variableId.GetHash();
}