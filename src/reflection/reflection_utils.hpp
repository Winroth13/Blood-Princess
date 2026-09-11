#pragma once

/*
*   Static Reflection System VERY HEAVILY BORROWED FROM
*   https://www.youtube.com/@ConnorDeMeyer.
*/

#define RegisterComponent(NewClassName) \
    using ClassName = NewClassName; \
    REGISTER_TYPE(NewClassName);

#define ExportProperty(MemberType, MemberName, ...) \
    MemberType MemberName = __VA_ARGS__; \
    REGISTER_MEMBER_PRIVATE(ClassName, MemberName);