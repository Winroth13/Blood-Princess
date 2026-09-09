#pragma once

#define SafeRelease(obj) do { if (obj) (obj)->Release(); (obj) = nullptr; } while (0)