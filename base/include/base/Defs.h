
#ifndef BASE_DEFS_H__
#define BASE_DEFS_H__


#ifndef __GNUC__
#define __attribute__(x)
#endif

//////////////////////////////////////////////////////////////////////////
// Join two variables
#define BASE_JOIN( X, Y ) BASE_DO_JOIN( X, Y )
#define BASE_DO_JOIN( X, Y ) BASE_DO_JOIN2(X,Y)
#define BASE_DO_JOIN2( X, Y ) X##Y


#undef MAX_PATH
#define MAX_PATH (260)


#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x) x
#endif


#ifdef __cplusplus
extern "C++"
{
template <typename T, int N>
char (&_base_countof_helper(T (&)[N]))[N];
#define BASE_COUNTOF(x) sizeof(_base_countof_helper(x))
}
#else
#define BASE_COUNTOF(x) (sizeof(x) / sizeof(x[0]))
#endif


#endif //BASE_DEFS_H__
