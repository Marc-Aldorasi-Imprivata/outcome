/* future_policy.ipp
Non-allocating constexpr future-promise
(C) 2015 Niall Douglas http://www.nedprod.com/
File Created: July 2015


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BOOST_SPINLOCK_FUTURE_NAME_POSTFIX
#error BOOST_SPINLOCK_FUTURE_NAME_POSTFIX needs to be defined
#endif
#define BOOST_SPINLOCK_GLUE2(a, b) a ## b
#define BOOST_SPINLOCK_GLUE(a, b) BOOST_SPINLOCK_GLUE2(a, b)
#ifndef BOOST_SPINLOCK_PROMISE_NAME
#define BOOST_SPINLOCK_PROMISE_NAME BOOST_SPINLOCK_GLUE(promise, BOOST_SPINLOCK_FUTURE_NAME_POSTFIX)
#endif
#ifndef BOOST_SPINLOCK_FUTURE_NAME
#define BOOST_SPINLOCK_FUTURE_NAME BOOST_SPINLOCK_GLUE(future, BOOST_SPINLOCK_FUTURE_NAME_POSTFIX)
#endif
#ifndef BOOST_SPINLOCK_SHARED_FUTURE_NAME
#define BOOST_SPINLOCK_SHARED_FUTURE_NAME BOOST_SPINLOCK_GLUE(shared_, BOOST_SPINLOCK_FUTURE_NAME)
#endif
#ifndef BOOST_SPINLOCK_FUTURE_POLICY_NAME
#define BOOST_SPINLOCK_FUTURE_POLICY_NAME BOOST_SPINLOCK_GLUE(BOOST_SPINLOCK_FUTURE_NAME, _policy)
#endif
#ifndef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME
#define BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME BOOST_SPINLOCK_GLUE(BOOST_SPINLOCK_SHARED_FUTURE_NAME, _policy)
#endif

namespace detail
{
  //! [future_policy]
  template<typename R> struct BOOST_SPINLOCK_FUTURE_POLICY_NAME;
  template<typename R> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME;
  template<> struct BOOST_SPINLOCK_FUTURE_POLICY_NAME<void>
  {
    typedef BOOST_SPINLOCK_FUTURE_POLICY_NAME<void> impl;
    typedef basic_monad<BOOST_SPINLOCK_FUTURE_POLICY_NAME> monad_type;
    // In a monad policy, this is identical to monad_type. Not here.
    typedef basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME> implementation_type;
    typedef void value_type;
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    typedef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE error_type;
#else
    typedef void error_type;
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    typedef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE exception_type;
#else
    typedef void exception_type;
#endif
    // This type is void for monad, here it points to our future type
    typedef basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME> *pointer_type;
    template<typename U> using rebind = basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME<U>>;
    template<typename U> using rebind_policy = BOOST_SPINLOCK_FUTURE_POLICY_NAME<U>;

    // Does getting this future's state consume it?
    BOOST_STATIC_CONSTEXPR bool is_consuming=true;
    // The type of future_errc to use for issuing errors
    typedef std::future_errc future_errc;
    // The type of future exception to use for issuing exceptions
    typedef std::future_error future_error;
    // The category of error code to use
    static const std::error_category &future_category() noexcept { return std::future_category(); }

    static BOOST_SPINLOCK_FUTURE_MSVC_HELP bool _throw_error(monad_errc ec)
    {
      switch(ec)
      {
        case monad_errc::already_set:
          throw future_error(future_errc::promise_already_satisfied);
        case monad_errc::no_state:
          throw future_error(future_errc::no_state);
        default:
          abort();
      }
    }
  protected:
    // Common preamble to the below
    template<bool is_consuming, class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _pre_get_value(U &&self)
    {
      self._check_validity();
#if defined(BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE) || defined(BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE)
      if(self.has_error() || self.has_exception())
      {
        // No decltype(auto) in C++ 11!
        decltype(detail::rebind_cast<monad_type>(self)) _self=detail::rebind_cast<monad_type>(self);
        typedef typename std::remove_const<typename std::decay<decltype(_self)>::type>::type &non_const_monad_type;
        non_const_monad_type _self_nc = const_cast<non_const_monad_type>(_self);
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
        if(self.has_error())
        {
          auto &category=_self._storage.error.category();
          //! \todo Is there any way of making which exception type to throw from an error_category user extensible? Seems daft this isn't in the STL :(
          if(category==std::future_category())
          {
            std::future_error e(_self._storage.error);
            if(is_consuming) _self_nc.clear();
            throw e;
          }
          /*else if(category==std::iostream_category())
          {
            std::ios_base::failure e(std::move(_self._storage.error));
            if(is_consuming) _self_nc.clear();
            throw e;
          }*/
          else
          {
            std::system_error e(_self._storage.error);
            if(is_consuming) _self_nc.clear();
            throw e;
          }
        }
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
        if(self.has_exception())
        {
          std::exception_ptr e(_self._storage.exception);
          if(is_consuming) _self_nc.clear();
          std::rethrow_exception(e);
        }
#endif
      }
#endif
    }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    template<bool is_consuming, class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type _get_error_impl(U &&self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(const_cast<U *>(&self));
      self._check_validity();
      if(self.has_error())
      {
        error_type ec(self._storage.error);
        if(is_consuming) self.clear();
        return ec;
      }
      if(self.has_exception())
        return error_type((int) monad_errc::exception_present, monad_category());
      return error_type();
    }
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    template<bool is_consuming, class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type _get_exception_impl(U &&self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(const_cast<U *>(&self));
      self._check_validity();
      if(!self.has_error() && !self.has_exception())
        return exception_type();
      if(self.has_error())
      {
        exception_type e(std::make_exception_ptr(std::system_error(self._storage.error)));
        if(is_consuming) self.clear();
        return e;
      }
      if(self.has_exception())
      {
        exception_type e(self._storage.exception);
        if(is_consuming) self.clear();
        return e;
      }
      return exception_type();
    }
#endif
  public:
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _get_value(implementation_type &self)
    {
      self.wait();
      implementation_type::lock_guard_type h(&self);
      _pre_get_value<is_consuming>(self);
      self.clear();
    }
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _get_value(const implementation_type &self)
    {
      self.wait();
      implementation_type::lock_guard_type h(const_cast<implementation_type *>(&self));
      _pre_get_value<is_consuming>(self);
      const_cast<implementation_type &>(self).clear();
    }
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _get_value(implementation_type &&self)
    {
      self.wait();
      implementation_type::lock_guard_type h(&self);
      _pre_get_value<is_consuming>(self);
      self.clear();
    }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type _get_error(const U &self)
    {
      return _get_error_impl<is_consuming>(self);
    }
#else
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type _get_error(const U &self);
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type _get_exception(const U &self)
    {
      return _get_exception_impl<is_consuming>(self);
    }
#else
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type _get_exception(const U &self);
#endif
    // Makes share() available on this future. Defined out of line as need shared_future_policy defined first.
    static inline BOOST_SPINLOCK_FUTURE_MSVC_HELP basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void>> _share(implementation_type &&self);
  private:
    // Disables implicit conversion from any other type of future
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP implementation_type _construct(basic_future<U> &&v);
  };
  template<typename R> struct BOOST_SPINLOCK_FUTURE_POLICY_NAME : public BOOST_SPINLOCK_FUTURE_POLICY_NAME<void>
  {
    typedef BOOST_SPINLOCK_FUTURE_POLICY_NAME<void> impl;
    typedef basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME> implementation_type;
    typedef R value_type;
    typedef basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME> *pointer_type;
    BOOST_STATIC_CONSTEXPR bool is_consuming=impl::is_consuming;

    // Called by get() &. Note we always return value_type by value.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP value_type _get_value(implementation_type &self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(&self);
      impl::_pre_get_value<is_consuming>(self);
      value_type v(std::move(self._storage.value));
      self.clear();
      return v;
    }
    // Called by get() const &. Note we always return value_type by value.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP value_type _get_value(const implementation_type &self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(const_cast<implementation_type *>(&self));
      impl::_pre_get_value<is_consuming>(self);
      value_type v(std::move(self._storage.value));
      const_cast<implementation_type &>(self).clear();
      return v;
    }
    // Called by get() &&. Note we always return value_type by value.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP value_type _get_value(implementation_type &&self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(&self);
      impl::_pre_get_value<is_consuming>(self);
      value_type v(std::move(self._storage.value));
      self.clear();
      return v;
    }
    static inline BOOST_SPINLOCK_FUTURE_MSVC_HELP basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>> _share(implementation_type &&self);
  };
  //! [future_policy]

  template<typename R> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME;
  template<> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void> : public BOOST_SPINLOCK_FUTURE_POLICY_NAME<void>
  {
    typedef BOOST_SPINLOCK_FUTURE_POLICY_NAME<void> impl;
    typedef basic_monad<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> monad_type;
    typedef basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> implementation_type;
    typedef void value_type;
    typedef basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> *pointer_type;
    template<typename U> using rebind = basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<U>>;
    template<typename U> using rebind_policy = BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<U>;

    BOOST_STATIC_CONSTEXPR bool is_consuming=false;

    static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _get_value(implementation_type &self)
    {
      self.wait();
      implementation_type::lock_guard_type h(&self);
      impl::_pre_get_value<is_consuming>(self);
    }
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _get_value(const implementation_type &self)
    {
      self.wait();
      implementation_type::lock_guard_type h(const_cast<implementation_type *>(&self));
      impl::_pre_get_value<is_consuming>(self);
    }
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP void _get_value(implementation_type &&self)
    {
      self.wait();
      implementation_type::lock_guard_type h(&self);
      impl::_pre_get_value<is_consuming>(self);
    }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type _get_error(const U &self)
    {
      return impl::_get_error_impl<is_consuming>(self);
    }
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    template<class U> static BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type _get_exception(const U &self)
    {
      return impl::_get_exception_impl<is_consuming>(self);
    }
#endif
    //! \todo Do not use reinterpret_cast to convert between consuming and non-consuming futures.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP implementation_type _construct(basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME<void>> &&v)
    {
      return implementation_type(reinterpret_cast<implementation_type &&>(v));
    }
    static inline BOOST_SPINLOCK_FUTURE_MSVC_HELP basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void>> _share(implementation_type &&self);
  };
  template<typename R> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME : public BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void>
  {
    typedef BOOST_SPINLOCK_FUTURE_POLICY_NAME<void> impl;
    typedef basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> implementation_type;
    typedef R value_type;
    typedef basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> *pointer_type;
    BOOST_STATIC_CONSTEXPR bool is_consuming=impl::is_consuming;

    // Called by get() &. Note we always return value_type by const lvalue ref.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP const value_type &_get_value(implementation_type &self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(&self);
      impl::_pre_get_value<is_consuming>(self);
      return self._storage.value;
    }
    // Called by get() const &. Note we always return value_type by const lvalue ref.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP const value_type &_get_value(const implementation_type &self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(const_cast<implementation_type *>(&self));
      impl::_pre_get_value<is_consuming>(self);
      return self._storage.value;
    }
    // Called by get() &&. Note we always return value_type by const lvalue ref.
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP const value_type &_get_value(implementation_type &&self)
    {
      self.wait();
      typename implementation_type::lock_guard_type h(&self);
      impl::_pre_get_value<is_consuming>(self);
      return self._storage.value;
    }
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP implementation_type _construct(basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>> &&v)
    {
      return implementation_type(reinterpret_cast<implementation_type &&>(v));
    }
    static inline BOOST_SPINLOCK_FUTURE_MSVC_HELP basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>> _share(implementation_type &&self);
  };
  inline basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void>> BOOST_SPINLOCK_FUTURE_POLICY_NAME<void>::_share(BOOST_SPINLOCK_FUTURE_POLICY_NAME<void>::implementation_type &&self)
  {
    return basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void>>(reinterpret_cast<basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<void>> &&>(self));
  }
  template<typename R> inline basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>> BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>::_share(typename BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>::implementation_type &&self)
  {
    return basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>>(reinterpret_cast<basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>> &&>(self));
  }
}

//! \ingroup future_promise
template<typename R> using BOOST_SPINLOCK_PROMISE_NAME = basic_promise<detail::BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>>;
//! \ingroup future_promise
template<typename R> using BOOST_SPINLOCK_FUTURE_NAME = basic_future<detail::BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>>;

#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME make_ready_ ## BOOST_SPINLOCK_FUTURE_NAME
//! \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_FUTURE_NAME<typename std::decay<R>::type> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(R &&v)
{
  return BOOST_SPINLOCK_FUTURE_NAME<typename std::decay<R>::type>(std::forward<R>(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME make_errored_ ## BOOST_SPINLOCK_FUTURE_NAME
//! \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(std::error_code v)
{
  return BOOST_SPINLOCK_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME make_exceptional_ ## BOOST_SPINLOCK_FUTURE_NAME
//! \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(std::exception_ptr v)
{
  return BOOST_SPINLOCK_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME

//! \ingroup future_promise
template<typename R> using BOOST_SPINLOCK_SHARED_FUTURE_NAME = shared_basic_future_ptr<basic_future<detail::BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>>>;

#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME make_ready_ ## BOOST_SPINLOCK_SHARED_FUTURE_NAME
//! \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_SHARED_FUTURE_NAME<typename std::decay<R>::type> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(R &&v)
{
  return BOOST_SPINLOCK_SHARED_FUTURE_NAME<typename std::decay<R>::type>(std::forward<R>(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME make_errored_ ## BOOST_SPINLOCK_SHARED_FUTURE_NAME
//! \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_SHARED_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(std::error_code v)
{
  return BOOST_SPINLOCK_SHARED_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME make_exceptional_ ## BOOST_SPINLOCK_SHARED_FUTURE_NAME
//! \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_SHARED_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(std::exception_ptr v)
{
  return BOOST_SPINLOCK_SHARED_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME

#undef BOOST_SPINLOCK_FUTURE_NAME_POSTFIX
#undef BOOST_SPINLOCK_GLUE
#undef BOOST_SPINLOCK_PROMISE_NAME
#undef BOOST_SPINLOCK_FUTURE_NAME
#undef BOOST_SPINLOCK_SHARED_FUTURE_NAME
#undef BOOST_SPINLOCK_FUTURE_POLICY_NAME
#undef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME
#undef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
#undef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE