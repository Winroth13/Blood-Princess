#pragma once

/*
*   Static Reflection System VERY HEAVILY BORROWED FROM
*   https://www.youtube.com/@ConnorDeMeyer.
*/

#define RegisterComponent(NewClassName) \
    using ClassName = NewClassName; \
    REGISTER_TYPE(NewClassName); \
    public: \
    virtual TypeId GetTypeId() const override { return TypeId::Create<NewClassName>(); } \
    private: \

#define ExportProperty(MemberType, MemberName, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName);

/*
*   Easy way to get a reference to any member variable inside a class or struct
*   given it's byte offset.
*/
template <typename T, typename Type>
T &GetMemberByOffset(Type *typePtr, size_t byteOffset)
{
    return *reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(typePtr) + byteOffset);
}