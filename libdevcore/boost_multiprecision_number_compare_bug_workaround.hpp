
// This is a copy of boost/multiprecision/detail/number_compare.hpp from boost 1.59 to replace buggy version from 1.58. 

#ifdef BOOST_MP_COMPARE_HPP
#error This bug workaround header must be included before original boost/multiprecision/detail/number_compare.hpp 
#endif

///////////////////////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MP_COMPARE_HPP
#define BOOST_MP_COMPARE_HPP

// A copy of boost/multiprecision/traits/is_backend.hpp
#ifndef BOOST_MP_IS_BACKEND_HPP
#define BOOST_MP_IS_BACKEND_HPP

#include <boost/mpl/has_xxx.hpp>
#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/multiprecision/detail/number_base.hpp>
#include <boost/multiprecision/detail/generic_interconvert.hpp>

namespace boost{ namespace multiprecision{  namespace detail{

   BOOST_MPL_HAS_XXX_TRAIT_DEF(signed_types)
   BOOST_MPL_HAS_XXX_TRAIT_DEF(unsigned_types)
   BOOST_MPL_HAS_XXX_TRAIT_DEF(float_types)

   template <class T>
   struct is_backend
   {
      static const bool value = has_signed_types<T>::value && has_unsigned_types<T>::value && has_float_types<T>::value;
   };

   template <class Backend>
   struct other_backend
   {
      typedef typename boost::conditional<
         boost::is_same<number<Backend>, number<Backend, et_on> >::value,
         number<Backend, et_off>, number<Backend, et_on> >::type type;
   };

   template <class B, class V>
   struct number_from_backend
   {
      typedef typename boost::conditional <
         boost::is_convertible<V, number<B> >::value,
         number<B>,
         typename other_backend<B>::type > ::type type;
   };

   template <bool b, class T, class U>
   struct is_first_backend_imp{ static const bool value = false; };
   template <class T, class U>
   struct is_first_backend_imp<true, T, U>{ static const bool value = is_convertible<U, number<T, et_on> >::value || is_convertible<U, number<T, et_off> >::value; };

   template <class T, class U>
   struct is_first_backend : is_first_backend_imp<is_backend<T>::value, T, U> {};

   template <bool b, class T, class U>
   struct is_second_backend_imp{ static const bool value = false; };
   template <class T, class U>
   struct is_second_backend_imp<true, T, U>{ static const bool value = is_convertible<T, number<U> >::value || is_convertible<T, number<U, et_off> >::value; };

   template <class T, class U>
   struct is_second_backend : is_second_backend_imp<is_backend<U>::value, T, U> {};

}
}
}

#endif // BOOST_MP_IS_BACKEND_HPP

//
// Comparison operators for number.
//

namespace boost{ namespace multiprecision{

namespace default_ops{

template <class B>
inline bool eval_eq(const B& a, const B& b)
{
   return a.compare(b) == 0;
}
template <class T, class U>
inline typename enable_if_c<boost::multiprecision::detail::is_first_backend<T, U>::value, bool>::type eval_eq(const T& a, const U& b)
{
   typename boost::multiprecision::detail::number_from_backend<T, U>::type t(b);
   return eval_eq(a, t.backend());
}
template <class T, class U>
inline typename enable_if_c<boost::multiprecision::detail::is_second_backend<T, U>::value, bool>::type eval_eq(const T& a, const U& b)
{
   typename boost::multiprecision::detail::number_from_backend<U, T>::type t(a);
   return eval_eq(t.backend(), b);
}

template <class B>
inline bool eval_lt(const B& a, const B& b)
{
   return a.compare(b) < 0;
}
template <class T, class U>
inline typename enable_if_c<boost::multiprecision::detail::is_first_backend<T, U>::value, bool>::type eval_lt(const T& a, const U& b)
{
   typename boost::multiprecision::detail::number_from_backend<T, U>::type t(b);
   return eval_lt(a, t.backend());
}
template <class T, class U>
inline typename enable_if_c<boost::multiprecision::detail::is_second_backend<T, U>::value, bool>::type eval_lt(const T& a, const U& b)
{
   typename boost::multiprecision::detail::number_from_backend<U, T>::type t(a);
   return eval_lt(t.backend(), b);
}

template <class B>
inline bool eval_gt(const B& a, const B& b)
{
   return a.compare(b) > 0;
}
template <class T, class U>
inline typename enable_if_c<boost::multiprecision::detail::is_first_backend<T, U>::value, bool>::type eval_gt(const T& a, const U& b)
{
   typename boost::multiprecision::detail::number_from_backend<T, U>::type t(b);
   return eval_gt(a, t.backend());
}
template <class T, class U>
inline typename enable_if_c<boost::multiprecision::detail::is_second_backend<T, U>::value, bool>::type eval_gt(const T& a, const U& b)
{
   typename boost::multiprecision::detail::number_from_backend<U, T>::type t(a);
   return eval_gt(t.backend(), b);
}

} // namespace default_ops

namespace detail{

template <class Num, class Val>
struct is_valid_mixed_compare : public mpl::false_ {};

template <class B, expression_template_option ET, class Val>
struct is_valid_mixed_compare<number<B, ET>, Val> : public is_convertible<Val, number<B, ET> > {};

template <class B, expression_template_option ET>
struct is_valid_mixed_compare<number<B, ET>, number<B, ET> > : public mpl::false_ {};

template <class B, expression_template_option ET, class tag, class Arg1, class Arg2, class Arg3, class Arg4>
struct is_valid_mixed_compare<number<B, ET>, expression<tag, Arg1, Arg2, Arg3, Arg4> > 
   : public mpl::bool_<is_convertible<expression<tag, Arg1, Arg2, Arg3, Arg4>, number<B, ET> >::value> {};

template <class tag, class Arg1, class Arg2, class Arg3, class Arg4, class B, expression_template_option ET>
struct is_valid_mixed_compare<expression<tag, Arg1, Arg2, Arg3, Arg4>, number<B, ET> > 
   : public mpl::bool_<is_convertible<expression<tag, Arg1, Arg2, Arg3, Arg4>, number<B, ET> >::value> {};

template <class Backend, expression_template_option ExpressionTemplates>
inline BOOST_CONSTEXPR typename boost::enable_if_c<number_category<Backend>::value != number_kind_floating_point, bool>::type is_unordered_value(const number<Backend, ExpressionTemplates>&)
{
   return false;
}
template <class Backend, expression_template_option ExpressionTemplates>
inline BOOST_CONSTEXPR typename boost::enable_if_c<number_category<Backend>::value == number_kind_floating_point, bool>::type is_unordered_value(const number<Backend, ExpressionTemplates>& a)
{
   using default_ops::eval_fpclassify;
   return eval_fpclassify(a.backend()) == FP_NAN;
}

template <class Arithmetic>
inline BOOST_CONSTEXPR typename boost::enable_if_c<number_category<Arithmetic>::value != number_kind_floating_point, bool>::type is_unordered_value(const Arithmetic&)
{
   return false;
}
template <class Arithmetic>
inline BOOST_CONSTEXPR typename boost::enable_if_c<number_category<Arithmetic>::value == number_kind_floating_point, bool>::type is_unordered_value(const Arithmetic& a)
{
   return (boost::math::isnan)(a);
}

template <class T, class U>
inline BOOST_CONSTEXPR bool is_unordered_comparison(const T& a, const U& b)
{
   return is_unordered_value(a) || is_unordered_value(b);
}

}

template <class Backend, expression_template_option ExpressionTemplates, class Backend2, expression_template_option ExpressionTemplates2>
inline bool operator == (const number<Backend, ExpressionTemplates>& a, const number<Backend2, ExpressionTemplates2>& b)
{
   using default_ops::eval_eq;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_eq(a.backend(), b.backend());
}
template <class Backend, expression_template_option ExpressionTemplates, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator == (const number<Backend, ExpressionTemplates>& a, const Arithmetic& b)
{
   using default_ops::eval_eq;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_eq(a.backend(), number<Backend, ExpressionTemplates>::canonical_value(b));
}
template <class Arithmetic, class Backend, expression_template_option ExpressionTemplates>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator == (const Arithmetic& a, const number<Backend, ExpressionTemplates>& b)
{
   using default_ops::eval_eq;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_eq(b.backend(), number<Backend, ExpressionTemplates>::canonical_value(a));
}
template <class Arithmetic, class Tag, class A1, class A2, class A3, class A4>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator == (const Arithmetic& a, const detail::expression<Tag, A1, A2, A3, A4>& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_eq;
   result_type t(b);
   if(detail::is_unordered_comparison(a, t)) return false;
   return eval_eq(t.backend(), result_type::canonical_value(a));
}
template <class Tag, class A1, class A2, class A3, class A4, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator == (const detail::expression<Tag, A1, A2, A3, A4>& a, const Arithmetic& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_eq;
   result_type t(a);
   if(detail::is_unordered_comparison(t, b)) return false;
   return eval_eq(t.backend(), result_type::canonical_value(b));
}
template <class Tag, class A1, class A2, class A3, class A4, class Tagb, class A1b, class A2b, class A3b, class A4b>
inline typename enable_if<is_same<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type>, bool>::type 
   operator == (const detail::expression<Tag, A1, A2, A3, A4>& a, const detail::expression<Tagb, A1b, A2b, A3b, A4b>& b)
{
   using default_ops::eval_eq;
   typename detail::expression<Tag, A1, A2, A3, A4>::result_type t(a);
   typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type t2(b);
   if(detail::is_unordered_comparison(t, t2)) return false;
   return eval_eq(t.backend(), t2.backend());
}

template <class Backend, expression_template_option ExpressionTemplates, class Backend2, expression_template_option ExpressionTemplates2>
inline bool operator != (const number<Backend, ExpressionTemplates>& a, const number<Backend2, ExpressionTemplates2>& b)
{
   using default_ops::eval_eq;
   if(detail::is_unordered_comparison(a, b)) return true;
   return !eval_eq(a.backend(), b.backend());
}
template <class Backend, expression_template_option ExpressionTemplates, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator != (const number<Backend, ExpressionTemplates>& a, const Arithmetic& b)
{
   using default_ops::eval_eq;
   if(detail::is_unordered_comparison(a, b)) return true;
   return !eval_eq(a.backend(), number<Backend, et_on>::canonical_value(b));
}
template <class Arithmetic, class Backend, expression_template_option ExpressionTemplates>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator != (const Arithmetic& a, const number<Backend, ExpressionTemplates>& b)
{
   using default_ops::eval_eq;
   if(detail::is_unordered_comparison(a, b)) return true;
   return !eval_eq(b.backend(), number<Backend, et_on>::canonical_value(a));
}
template <class Arithmetic, class Tag, class A1, class A2, class A3, class A4>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator != (const Arithmetic& a, const detail::expression<Tag, A1, A2, A3, A4>& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_eq;
   result_type t(b);
   if(detail::is_unordered_comparison(a, t)) return true;
   return !eval_eq(t.backend(), result_type::canonical_value(a));
}
template <class Tag, class A1, class A2, class A3, class A4, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator != (const detail::expression<Tag, A1, A2, A3, A4>& a, const Arithmetic& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_eq;
   result_type t(a);
   if(detail::is_unordered_comparison(t, b)) return true;
   return !eval_eq(t.backend(), result_type::canonical_value(b));
}
template <class Tag, class A1, class A2, class A3, class A4, class Tagb, class A1b, class A2b, class A3b, class A4b>
inline typename enable_if<is_same<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type>, bool>::type 
   operator != (const detail::expression<Tag, A1, A2, A3, A4>& a, const detail::expression<Tagb, A1b, A2b, A3b, A4b>& b)
{
   using default_ops::eval_eq;
   typename detail::expression<Tag, A1, A2, A3, A4>::result_type t(a);
   typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type t2(b);
   if(detail::is_unordered_comparison(t, t2)) return true;
   return !eval_eq(t.backend(), t2.backend());
}

template <class Backend, expression_template_option ExpressionTemplates, class Backend2, expression_template_option ExpressionTemplates2>
inline bool operator < (const number<Backend, ExpressionTemplates>& a, const number<Backend2, ExpressionTemplates2>& b)
{
   using default_ops::eval_lt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_lt(a.backend(), b.backend());
}
template <class Backend, expression_template_option ExpressionTemplates, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator < (const number<Backend, ExpressionTemplates>& a, const Arithmetic& b)
{
   using default_ops::eval_lt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_lt(a.backend(), number<Backend, ExpressionTemplates>::canonical_value(b));
}
template <class Arithmetic, class Backend, expression_template_option ExpressionTemplates>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator < (const Arithmetic& a, const number<Backend, ExpressionTemplates>& b)
{
   using default_ops::eval_gt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_gt(b.backend(), number<Backend, ExpressionTemplates>::canonical_value(a));
}
template <class Arithmetic, class Tag, class A1, class A2, class A3, class A4>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator < (const Arithmetic& a, const detail::expression<Tag, A1, A2, A3, A4>& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_gt;
   result_type t(b);
   if(detail::is_unordered_comparison(a, t)) return false;
   return eval_gt(t.backend(), result_type::canonical_value(a));
}
template <class Tag, class A1, class A2, class A3, class A4, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator < (const detail::expression<Tag, A1, A2, A3, A4>& a, const Arithmetic& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_lt;
   result_type t(a);
   if(detail::is_unordered_comparison(t, b)) return false;
   return eval_lt(t.backend(), result_type::canonical_value(b));
}
template <class Tag, class A1, class A2, class A3, class A4, class Tagb, class A1b, class A2b, class A3b, class A4b>
inline typename enable_if<is_same<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type>, bool>::type 
   operator < (const detail::expression<Tag, A1, A2, A3, A4>& a, const detail::expression<Tagb, A1b, A2b, A3b, A4b>& b)
{
   using default_ops::eval_lt;
   typename detail::expression<Tag, A1, A2, A3, A4>::result_type t(a);
   typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type t2(b);
   if(detail::is_unordered_comparison(t, t2)) return false;
   return eval_lt(t.backend(), t2.backend());
}

template <class Backend, expression_template_option ExpressionTemplates, class Backend2, expression_template_option ExpressionTemplates2>
inline bool operator > (const number<Backend, ExpressionTemplates>& a, const number<Backend2, ExpressionTemplates2>& b)
{
   using default_ops::eval_gt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_gt(a.backend(), b.backend());
}
template <class Backend, expression_template_option ExpressionTemplates, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator > (const number<Backend, ExpressionTemplates>& a, const Arithmetic& b)
{
   using default_ops::eval_gt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_gt(a.backend(), number<Backend, ExpressionTemplates>::canonical_value(b));
}
template <class Arithmetic, class Backend, expression_template_option ExpressionTemplates>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator > (const Arithmetic& a, const number<Backend, ExpressionTemplates>& b)
{
   using default_ops::eval_lt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return eval_lt(b.backend(), number<Backend, ExpressionTemplates>::canonical_value(a));
}
template <class Arithmetic, class Tag, class A1, class A2, class A3, class A4>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator > (const Arithmetic& a, const detail::expression<Tag, A1, A2, A3, A4>& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_lt;
   result_type t(b);
   if(detail::is_unordered_comparison(a, t)) return false;
   return a > t;
}
template <class Tag, class A1, class A2, class A3, class A4, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator > (const detail::expression<Tag, A1, A2, A3, A4>& a, const Arithmetic& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_gt;
   result_type t(a);
   if(detail::is_unordered_comparison(t, b)) return false;
   return t > b;
}
template <class Tag, class A1, class A2, class A3, class A4, class Tagb, class A1b, class A2b, class A3b, class A4b>
inline typename enable_if<is_same<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type>, bool>::type 
   operator > (const detail::expression<Tag, A1, A2, A3, A4>& a, const detail::expression<Tagb, A1b, A2b, A3b, A4b>& b)
{
   using default_ops::eval_gt;
   typename detail::expression<Tag, A1, A2, A3, A4>::result_type t(a);
   typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type t2(b);
   if(detail::is_unordered_comparison(t, t2)) return false;
   return t > t2;
}

template <class Backend, expression_template_option ExpressionTemplates, class Backend2, expression_template_option ExpressionTemplates2>
inline bool operator <= (const number<Backend, ExpressionTemplates>& a, const number<Backend2, ExpressionTemplates2>& b)
{
   using default_ops::eval_gt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return !eval_gt(a.backend(), b.backend());
}
template <class Backend, expression_template_option ExpressionTemplates, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator <= (const number<Backend, ExpressionTemplates>& a, const Arithmetic& b)
{
   using default_ops::eval_gt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return !eval_gt(a.backend(), number<Backend, ExpressionTemplates>::canonical_value(b));
}
template <class Arithmetic, class Backend, expression_template_option ExpressionTemplates>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator <= (const Arithmetic& a, const number<Backend, ExpressionTemplates>& b)
{
   using default_ops::eval_lt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return !eval_lt(b.backend(), number<Backend, ExpressionTemplates>::canonical_value(a));
}
template <class Arithmetic, class Tag, class A1, class A2, class A3, class A4>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator <= (const Arithmetic& a, const detail::expression<Tag, A1, A2, A3, A4>& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_lt;
   if(detail::is_unordered_value(a) || detail::is_unordered_value(b))
      return false;
   result_type t(b);
   if(detail::is_unordered_comparison(a, t)) return false;
   return !eval_lt(t.backend(), result_type::canonical_value(a));
}
template <class Tag, class A1, class A2, class A3, class A4, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator <= (const detail::expression<Tag, A1, A2, A3, A4>& a, const Arithmetic& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_gt;
   result_type t(a);
   if(detail::is_unordered_comparison(t, b)) return false;
   return !eval_gt(t.backend(), result_type::canonical_value(b));
}
template <class Tag, class A1, class A2, class A3, class A4, class Tagb, class A1b, class A2b, class A3b, class A4b>
inline typename enable_if<is_same<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type>, bool>::type 
   operator <= (const detail::expression<Tag, A1, A2, A3, A4>& a, const detail::expression<Tagb, A1b, A2b, A3b, A4b>& b)
{
   using default_ops::eval_gt;
   typename detail::expression<Tag, A1, A2, A3, A4>::result_type t(a);
   typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type t2(b);
   if(detail::is_unordered_comparison(t, t2)) return false;
   return !eval_gt(t.backend(), t2.backend());
}

template <class Backend, expression_template_option ExpressionTemplates, class Backend2, expression_template_option ExpressionTemplates2>
inline bool operator >= (const number<Backend, ExpressionTemplates>& a, const number<Backend2, ExpressionTemplates2>& b)
{
   using default_ops::eval_lt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return !eval_lt(a.backend(), b.backend());
}
template <class Backend, expression_template_option ExpressionTemplates, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator >= (const number<Backend, ExpressionTemplates>& a, const Arithmetic& b)
{
   using default_ops::eval_lt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return !eval_lt(a.backend(), number<Backend, ExpressionTemplates>::canonical_value(b));
}
template <class Arithmetic, class Backend, expression_template_option ExpressionTemplates>
inline typename enable_if_c<detail::is_valid_mixed_compare<number<Backend, ExpressionTemplates>, Arithmetic>::value, bool>::type 
   operator >= (const Arithmetic& a, const number<Backend, ExpressionTemplates>& b)
{
   using default_ops::eval_gt;
   if(detail::is_unordered_comparison(a, b)) return false;
   return !eval_gt(b.backend(), number<Backend, ExpressionTemplates>::canonical_value(a));
}
template <class Arithmetic, class Tag, class A1, class A2, class A3, class A4>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator >= (const Arithmetic& a, const detail::expression<Tag, A1, A2, A3, A4>& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_gt;
   result_type t(b);
   if(detail::is_unordered_comparison(a, t)) return false;
   return !eval_gt(t.backend(), result_type::canonical_value(a));
}
template <class Tag, class A1, class A2, class A3, class A4, class Arithmetic>
inline typename enable_if_c<detail::is_valid_mixed_compare<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, Arithmetic>::value, bool>::type 
   operator >= (const detail::expression<Tag, A1, A2, A3, A4>& a, const Arithmetic& b)
{
   typedef typename detail::expression<Tag, A1, A2, A3, A4>::result_type result_type;
   using default_ops::eval_lt;
   result_type t(a);
   if(detail::is_unordered_comparison(t, b)) return false;
   return !eval_lt(t.backend(), result_type::canonical_value(b));
}
template <class Tag, class A1, class A2, class A3, class A4, class Tagb, class A1b, class A2b, class A3b, class A4b>
inline typename enable_if<is_same<typename detail::expression<Tag, A1, A2, A3, A4>::result_type, typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type>, bool>::type 
   operator >= (const detail::expression<Tag, A1, A2, A3, A4>& a, const detail::expression<Tagb, A1b, A2b, A3b, A4b>& b)
{
   using default_ops::eval_lt;
   typename detail::expression<Tag, A1, A2, A3, A4>::result_type t(a);
   typename detail::expression<Tagb, A1b, A2b, A3b, A4b>::result_type t2(b);
   if(detail::is_unordered_comparison(t, t2)) return false;
   return !eval_lt(t.backend(), t2.backend());
}


}} // namespaces

#endif // BOOST_MP_COMPARE_HPP
