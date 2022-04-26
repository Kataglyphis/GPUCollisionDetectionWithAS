#pragma once


// Windows / Linux stuff
#ifdef _WIN32
#define FILE_SEPARATOR "\\"
#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak();
#else
#define DEBUG_BREAKDebugBreak();
#endif

#else // not _WIN32
#include <signal.h>
#define FILE_SEPARATOR '/'
#define DEBUG_BREAK raise(SIGTRAP);

#endif // _WIN32


// Error checking on vulkan function calls
#define ASSERT_VULKAN(val)\
            if(val!=VK_SUCCESS) {\
                DEBUG_BREAK \
            }



#define NOT_YET_IMPLEMENTED throw std::runtime_error("Not yet implemented!");

#define VK_LOAD(FUNCTION_NAME) PFN_##FUNCTION_NAME p##FUNCTION_NAME = (PFN_##FUNCTION_NAME) glfwGetInstanceProcAddress(context->instance, #FUNCTION_NAME);

template <class integral>
constexpr integral alignUp(integral x, size_t a) noexcept
{
    return integral((x + (integral(a) - 1)) & ~integral(a - 1));
}