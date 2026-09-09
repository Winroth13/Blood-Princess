Code Standard
Variables: camelCase, this instead of m_
Classes: PascalCase, struct is class
Files: snake_case, folders are files
Constants: SCREAMING_SNAKE_CASE
Functions: PascalCase
Enums: PascalCase, SCREAMING_SNAKE_CASE for entries
Get and Set rather than direct access in classes.
Structs do not have Get and Set.
Is- and Has- for bool-functions.
{ on a new line.
Single-line Get and Set in the .hpp-file.
.hpp, not .h
No custom namespaces.
Using is okay in .cpp, but not for std.
Preferably avoid raw pointers, use smart pointers.
No com-pointers.
Private variables and functions first in classes.
Return error values, do not throw exceptions.
Error messages in headers.
#pragma once as header guard.
Forward declare rather than include in .hpp.
Include for DirectX.
// for comments.
Capital letters in names in comments and logger.
* and & on rihgt side of variable name.

GitHub Commits
Try to keep up with our documentation of classes.
Commits to main must build.