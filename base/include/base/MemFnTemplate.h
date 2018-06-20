

#if FUNCTION_ARGS_NUMBER == 0
#  define FUNCTION_COMMA
#else
#  define FUNCTION_COMMA ,
#endif // FUNCTION_ARGS_NUMBER > 0


#define MemFunctorX           BASE_JOIN(MemFunctor, FUNCTION_ARGS_NUMBER)
#define BindMemFunctorX       BASE_JOIN(BindMemFunctor, FUNCTION_ARGS_NUMBER)


namespace Base {

namespace Detail {

////////////////////////////////////////////////////////////////////////////////

template <class R, class F>
struct MemFunctorX
{
    typedef R result_type;

    F f;

    MemFunctorX(F func)
        : f(func)
    {
    }

    template <class T FUNCTION_COMMA FUNCTION_ARGS_CLASS_DECL>
    result_type operator()(T* o FUNCTION_COMMA FUNCTION_ARGS_VAR_DECL)
    {
        return (o->*f)(FUNCTION_ARGS_VAR);
    }

    bool operator==(MemFunctorX const& rhs) const
    {
        return f == rhs.f;
    }

    bool operator!=(MemFunctorX const& rhs) const
    {
        return f != rhs.f;
    }
};

template <class T, class R FUNCTION_COMMA FUNCTION_ARGS_CLASS_DECL>
struct MemFunctorType<R (T::*)(FUNCTION_ARGS_CLASS)>
{
    typedef R (T::*F)(FUNCTION_ARGS_CLASS);
    typedef MemFunctorX<R, F> Type;
};


template <class R, class F, class B>
struct BindMemFunctorX
{
    typedef R result_type;

    F f;
    B b;

    BindMemFunctorX(F func, B bi)
        : f(func)
        , b(bi)
    {
    }

#if FUNCTION_ARGS_NUMBER != 0
    template <FUNCTION_ARGS_CLASS_DECL>
#endif
    result_type operator()(FUNCTION_ARGS_VAR_DECL)
    {
        return (b->*f)(FUNCTION_ARGS_VAR);
    }

    bool operator==(BindMemFunctorX const& rhs) const
    {
        return f == rhs.f;
    }

    bool operator!=(BindMemFunctorX const& rhs) const
    {
        return f != rhs.f;
    }
};

template <class B, class T, class R FUNCTION_COMMA FUNCTION_ARGS_CLASS_DECL>
struct BindMemFunctorType<B, R (T::*)(FUNCTION_ARGS_CLASS)>
{
    typedef R (T::*F)(FUNCTION_ARGS_CLASS);
    typedef BindMemFunctorX<R, F, B> Type;
};



} // namespace Detail

////////////////////////////////////////////////////////////////////////////////

} // namespace Base


#undef FUNCTION_COMMA

#undef MemFunctorX
#undef BindMemFunctorX


