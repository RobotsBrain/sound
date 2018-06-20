#ifndef __BASE_MEMORY_SINGLETON_H__
#define __BASE_MEMORY_SINGLETON_H__


namespace Base {


// T must be: no-throw default constructible and no-throw destructible
template <typename T>
struct TSingletonDefault
{
  private:
	struct object_creator
	{
	  // This constructor does nothing more than ensure that Instance()
	  //  is called before main() begins, thus creating the static
	  //  T object before multithreading race issues can come up.
	  object_creator() { TSingletonDefault<T>::Instance(); }
	  inline void do_nothing() const { }
	};
	static object_creator create_object;

	TSingletonDefault();

  public:
	typedef T object_type;

	// If, at any point (in user code), TSingletonDefault<T>::Instance()
	//  is called, then the following function is instantiated.
	static object_type & Instance()
	{
	  // This is the object that we return a reference to.
	  // It is guaranteed to be created before main() begins because of
	  //  the next line.
	  static object_type obj;

	  // The following line does nothing else than force the instantiation
	  //  of TSingletonDefault<T>::create_object, whose constructor is
	  //  called before main() begins.
	  create_object.do_nothing();

	  return obj;
	}
};
template <typename T>
typename TSingletonDefault<T>::object_creator
TSingletonDefault<T>::create_object;

} // namespace Base

#endif // __BASE_MEMORY_SINGLETON_H__
