/* A very simple result type
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (59 commits)
File Created: June 2017


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
(See accompanying file Licence.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef OUTCOME_BOOST_RESULT_HPP
#define OUTCOME_BOOST_RESULT_HPP

#include "std_result.hpp"

#include "boost/system.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  // Customise _set_error_is_errno
  template <class State> constexpr inline void _set_error_is_errno(State &state, const boost::error_code &error)
  {
    if(error.category() == boost::generic_category()
#ifndef _WIN32
       || error.category() == boost::system_category()
#endif
       )
    {
      state._status |= status_error_is_errno;
    }
  }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const boost::error_condition &error)
  {
    if(error.category() == boost::generic_category()
#ifndef _WIN32
       || error.category() == boost::system_category()
#endif
       )
    {
      state._status |= status_error_is_errno;
    }
  }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const boost::errc & /*unused*/) { state._status |= status_error_is_errno; }

}  // namespace detail

//! Namespace for policies
namespace policy
{
  namespace detail
  {
    /* Pass through `make_error_code` function for anything implicitly convertible to `boost::error_code`.
    \requires `T` is implicitly convertible to `boost::error_code`.
    */
    OUTCOME_TEMPLATE(class T)
    OUTCOME_TREQUIRES(OUTCOME_TPRED(std::is_convertible<T, boost::error_code>::value))
    constexpr inline decltype(auto) make_error_code(T &&v, error_code_passthrough /*unused*/ = {}) { return std::forward<T>(v); }

    /* Pass through `make_exception_ptr` function for `boost::exception_ptr`.
    */
    inline boost::exception_ptr make_exception_ptr(boost::exception_ptr v) { return v; }

    struct boost_enum_overload_tag
    {
    };
  }  // namespace detail

  inline void throw_as_system_error_with_payload(boost::error_code error) { OUTCOME_THROW_EXCEPTION(boost::system_error(error)); }
  OUTCOME_TEMPLATE(class Error)
  OUTCOME_TREQUIRES(OUTCOME_TPRED(boost::is_error_code_enum<std::decay_t<Error>>::value || boost::is_error_condition_enum<std::decay_t<Error>>::value))
  inline void throw_as_system_error_with_payload(Error &&error, detail::boost_enum_overload_tag = detail::boost_enum_overload_tag()) { OUTCOME_THROW_EXCEPTION(boost::system_error(error_code(error))); }
}  // namespace policy

//! Namespace for traits
namespace trait
{
  namespace detail
  {
    template <> struct has_error_code<boost::error_code, void>
    {
      static constexpr bool value = true;
    };
    template <class T> struct has_error_code<T, boost::error_code>
    {
      static constexpr bool value = true;
    };

    template <> struct has_exception_ptr<boost::exception_ptr, void>
    {
      static constexpr bool value = true;
    };
    template <class T> struct has_exception_ptr<T, boost::exception_ptr>
    {
      static constexpr bool value = true;
    };
  }  // namespace detail

  // std::error_code and std::exception_ptr are error types
  template <> struct is_error_type<boost::error_code>
  {
    static constexpr bool value = true;
  };
  template <> struct is_error_type<boost::exception_ptr>
  {
    static constexpr bool value = true;
  };
  // For boost::error_code, boost::is_error_condition_enum<> is the trait we want.
  template <class Enum> struct is_error_type_enum<boost::error_code, Enum>
  {
    static constexpr bool value = boost::is_error_condition_enum<Enum>::value;
  };

}  // namespace trait


/*! `basic_result` defaulted to use `boost::error_code` and a `NoValuePolicy` appropriate for `boost` types.

`NoValuePolicy` defaults to a policy selected according to the characteristics of type `S`:

1. If `.value()` called when there is no `value_type` but there is an `error_type`:
- If \verbatim {{<api "success_failure/#unexposed-entity-outcome-v2-xxx-trait-has-error-code-v" "trait::has_error_code_v<S>">}} \end is true,
then `throw boost::system_error(error()|make_error_code(error()))` [\verbatim {{<api "policies/result_error_code_throw_as_system_error" "policy::error_code_throw_as_system_error<S>">}} \end]
- If \verbatim {{<api "success_failure/#unexposed-entity-outcome-v2-xxx-trait-has-exception-ptr-v" "trait::has_exception_ptr_v<S>">}} \end is true, then `std::rethrow_exception(error()|make_exception_ptr(error()))`
[\verbatim {{<api "policies/result_exception_ptr_rethrow/" "policy::exception_ptr_rethrow<R, S, void>">}} \end]
- If `S` is `void`, call `std::terminate()` [\verbatim {{<api "policies/terminate/" "policy::terminate">}} \end]
- If `S` is none of the above, then it is undefined behaviour [\verbatim {{<api "policies/all_narrow/" "policy::all_narrow">}} \end]
2. If `.error()` called when there is no `error_type`:
- If `trait::has_error_code_v<S>`, or if `trait::has_exception_ptr_v<S>`,
or if `S` is `void`, do `throw bad_result_access()`
- If `S` is none of the above, then it is undefined behaviour [`policy::all_narrow`]
*/
template <class R, class S = boost::error_code, class NoValuePolicy = policy::default_policy<R, S, void>>  //
using boost_result = basic_result<R, S, NoValuePolicy>;

/*! An "unchecked" edition of `result<T, E>` which does no special handling of specific `E` types at all.
Attempting to access `T` when there is an `E` results in nothing happening at all, it is treated with a narrow
contract (i.e. undefined behaviour).
*/
template <class R, class S = boost::error_code> using boost_unchecked = boost_result<R, S, policy::all_narrow>;

/*! A "checked" edition of `result<T, E>` which resembles fairly closely a `std::expected<T, E>`.
Attempting to access `T` when there is an `E` results in `bad_result_access<E>` being thrown. Nothing else.

Note that this approximates the proposed `expected<T, E>` up for standardisation, see the FAQ for more
detail.
*/
template <class R, class S = boost::error_code> using boost_checked = boost_result<R, S, policy::throw_bad_result_access<S>>;

OUTCOME_V2_NAMESPACE_END

#endif