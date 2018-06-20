#ifndef __BASE_MEMBER_FUNCTOR_H__
#define __BASE_MEMBER_FUNCTOR_H__


#include "Defs.h"


namespace Base {
namespace Detail {

template <class F> struct MemFunctorType {};
template <class F, class B> struct BindMemFunctorType {};

} // namespace Detail
} //namespace Base


// Args 0
#define FUNCTION_ARGS_NUMBER     0
#define FUNCTION_ARGS_CLASS_DECL
#define FUNCTION_ARGS_VAR_DECL
#define FUNCTION_ARGS_CLASS
#define FUNCTION_ARGS_VAR
#include "MemFnTemplate.h"
#undef FUNCTION_ARGS_NUMBER
#undef FUNCTION_ARGS_CLASS_DECL
#undef FUNCTION_ARGS_VAR_DECL
#undef FUNCTION_ARGS_CLASS
#undef FUNCTION_ARGS_VAR

// Args 1
#define FUNCTION_ARGS_NUMBER     1
#define FUNCTION_ARGS_CLASS_DECL class T1
#define FUNCTION_ARGS_VAR_DECL   T1 a1
#define FUNCTION_ARGS_CLASS      T1
#define FUNCTION_ARGS_VAR        a1
#include "MemFnTemplate.h"
#undef FUNCTION_ARGS_NUMBER
#undef FUNCTION_ARGS_CLASS_DECL
#undef FUNCTION_ARGS_VAR_DECL
#undef FUNCTION_ARGS_CLASS
#undef FUNCTION_ARGS_VAR

// Args 2
#define FUNCTION_ARGS_NUMBER     2
#define FUNCTION_ARGS_CLASS_DECL class T1, class T2
#define FUNCTION_ARGS_VAR_DECL   T1 a1, T2 a2
#define FUNCTION_ARGS_CLASS      T1, T2
#define FUNCTION_ARGS_VAR        a1, a2
#include "MemFnTemplate.h"
#undef FUNCTION_ARGS_NUMBER
#undef FUNCTION_ARGS_CLASS_DECL
#undef FUNCTION_ARGS_VAR_DECL
#undef FUNCTION_ARGS_CLASS
#undef FUNCTION_ARGS_VAR

// Args 3
#define FUNCTION_ARGS_NUMBER     3
#define FUNCTION_ARGS_CLASS_DECL class T1, class T2, class T3
#define FUNCTION_ARGS_VAR_DECL   T1 a1, T2 a2, T3 a3
#define FUNCTION_ARGS_CLASS      T1, T2, T3
#define FUNCTION_ARGS_VAR        a1, a2, a3
#include "MemFnTemplate.h"
#undef FUNCTION_ARGS_NUMBER
#undef FUNCTION_ARGS_CLASS_DECL
#undef FUNCTION_ARGS_VAR_DECL
#undef FUNCTION_ARGS_CLASS
#undef FUNCTION_ARGS_VAR

// Args 4
#define FUNCTION_ARGS_NUMBER     4
#define FUNCTION_ARGS_CLASS_DECL class T1, class T2, class T3, class T4
#define FUNCTION_ARGS_VAR_DECL   T1 a1, T2 a2, T3 a3, T4 a4
#define FUNCTION_ARGS_CLASS      T1, T2, T3, T4
#define FUNCTION_ARGS_VAR        a1, a2, a3, a4
#include "MemFnTemplate.h"
#undef FUNCTION_ARGS_NUMBER
#undef FUNCTION_ARGS_CLASS_DECL
#undef FUNCTION_ARGS_VAR_DECL
#undef FUNCTION_ARGS_CLASS
#undef FUNCTION_ARGS_VAR

// Args 5
#define FUNCTION_ARGS_NUMBER     5
#define FUNCTION_ARGS_CLASS_DECL class T1, class T2, class T3, class T4, class T5
#define FUNCTION_ARGS_VAR_DECL   T1 a1, T2 a2, T3 a3, T4 a4, T5 a5
#define FUNCTION_ARGS_CLASS      T1, T2, T3, T4, T5
#define FUNCTION_ARGS_VAR        a1, a2, a3, a4, a5
#include "MemFnTemplate.h"
#undef FUNCTION_ARGS_NUMBER
#undef FUNCTION_ARGS_CLASS_DECL
#undef FUNCTION_ARGS_VAR_DECL
#undef FUNCTION_ARGS_CLASS
#undef FUNCTION_ARGS_VAR


namespace Base {


template <class F> inline typename Detail::MemFunctorType<F>::Type MemFunc(F f)
{
    return typename Detail::MemFunctorType<F>::Type(f);
}

template <class F, class O> inline typename Detail::BindMemFunctorType<O, F>::Type MemFunc(F f, O o)
{
    return typename Detail::BindMemFunctorType<O, F>::Type(f, o);
}

} //namespace Base

#endif // __BASE_MEMBER_FUNCTOR_H__

