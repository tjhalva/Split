﻿#ifndef SPLIT_H
#define SPLIT_H

// Compiler Validation
#if __cplusplus < 201103L
#error This compilation unit requires at least a C++11 compliant compilier.
#endif

// System Headers
#include <algorithm>
#include <string>
#include <tuple>

#ifdef __cpp_lib_integer_sequence
#include <utility>
#endif

//! This is simply pulling in the integer sequence components that exist in
//! C++14. If C++14 isn't enabled, they are implemented below in C++11.
#ifndef __cpp_lib_integer_sequence

// Taken from libstdc++-v3
// Revision: 210470

namespace tjh
{
namespace std_impl
{
  // Stores a tuple of indices. Used by tuple and pair, and by bind() to
  // extract the elements in a tuple.
  template<size_t... _Indexes>
  struct _Index_tuple
  {
    typedef _Index_tuple<_Indexes..., sizeof...(_Indexes)> __next;
  };

  // Builds an _Index_tuple<0, 1, 2, ..., _Num-1>.
  template<size_t _Num>
  struct _Build_index_tuple
  {
    typedef typename _Build_index_tuple<_Num - 1>::__type::__next __type;
  };

  template<>
  struct _Build_index_tuple<0>
  {
    typedef _Index_tuple<> __type;
  };

  /// Class template integer_sequence
  template<typename _Tp, _Tp... _Idx>
  struct integer_sequence
  {
      typedef _Tp value_type;
      static constexpr size_t size() { return sizeof...(_Idx); }
  };

  template<typename _Tp, _Tp _Num, typename _ISeq = typename _Build_index_tuple<_Num>::__type>
  struct _Make_integer_sequence;

  template<typename _Tp, _Tp _Num, size_t... _Idx>
  struct _Make_integer_sequence<_Tp, _Num, _Index_tuple<_Idx...>>
  {
    static_assert( _Num >= 0, "Cannot make integer sequence of negative length" );

    typedef integer_sequence<_Tp, static_cast<_Tp>(_Idx)...> __type;
  };

  /// Alias template make_integer_sequence
  template<typename _Tp, _Tp _Num>
  using make_integer_sequence = typename _Make_integer_sequence<_Tp, _Num>::__type;

  /// Alias template index_sequence
  template<size_t... _Idx>
  using index_sequence = integer_sequence<size_t, _Idx...>;

  /// Alias template make_index_sequence
  template<size_t _Num>
  using make_index_sequence = make_integer_sequence<size_t, _Num>;

} // std_impl
} // tjh

//! Add the necessary sequences to the std namespace to support future
//! compatibility with C++14.
namespace std
{
  /// Alias template integer_sequence
  template<typename _Tp, _Tp... _Idx>
  using integer_sequence = tjh::std_impl::integer_sequence<_Tp, _Idx...>;

  /// Alias template make_integer_sequence
  template<typename _Tp, _Tp _Num>
  using make_integer_sequence = tjh::std_impl::make_integer_sequence<_Tp, _Num>;

  /// Alias template index_sequence
  template<size_t... _Idx>
  using index_sequence = tjh::std_impl::integer_sequence<size_t, _Idx...>;

  /// Alias template make_index_sequence
  template<size_t _Num>
  using make_index_sequence = tjh::std_impl::make_integer_sequence<size_t, _Num>;

}// end std

#endif

namespace tjh
{
  namespace detail
  {
    //! A dynamic tuple type generator of length N.
    //! @tparam T The type of element at each index.
    //! @tparam N The number of indexes to generate.
    //! @note This implementation may be inefficient for large N if
    //!   depending on how make_index_sequence<T> is implemented.
    template <typename T, size_t N>
    class generate_tuple_type
    {
        template <typename = std::make_index_sequence<N>>
        struct impl;
    
        template <size_t... Is>
        struct impl<std::index_sequence<Is...>>
        {
          template <size_t>
          using wrap = T;
    
          using type = std::tuple<wrap<Is>...>;
        };
    
    public:
        using type = typename impl<>::type;
    };  
      
    //! Helper function which utilizes a functor that computes the
    //! desire value of at a given index, when supplied with that index
    //! @tparam F A callable object which takes an integral index as a single parameter.
    //! @tparam Is Variadic number of indexes to conjure.
    template <typename F, size_t... Is>
    auto generate_tuple_impl(F func, std::index_sequence<Is...> ) -> 
      decltype(std::make_tuple(func(Is)...))
    {
      return std::make_tuple(func(Is)...);
    }

    //! A method which generates a tuple of N types from the supplied
    //! generator function F.
    //! @tparam N The number of types to generate.
    //! @tparam F Generator callable object used to seed tuple values.
    template <size_t N, typename F>
    auto generate_tuple(F func) ->
      decltype(generate_tuple_impl(func, std::make_index_sequence<N>{}))
    {
      return generate_tuple_impl(func, std::make_index_sequence<N>{});
    }    
    
    #if defined(__GNUC__) || defined(__GNUG__)
    
    //! Helper method which reverses the index offset of tuple types.
    template<typename T, size_t... I>
    auto invert_impl(T&& t, std::index_sequence<I...>) ->
      decltype(std::make_tuple(std::get<sizeof...(I) - 1 - I>(std::forward<T>(t))...))
    {
      return std::make_tuple(
        std::get<sizeof...(I) - 1 - I>(std::forward<T>(t))...);
    }
    
    //! Method which inverts the ordering of types and values contained
    //! within the suplied tuple.
    template<typename T>
    auto invert(T&& tuple) ->
      decltype(invert_impl(std::forward<T>(tuple), std::make_index_sequence<std::tuple_size<T>::value>()))
    {
      return invert_impl(
        std::forward<T>(tuple),
        std::make_index_sequence<std::tuple_size<T>::value>());
    }
    
    #endif
    
  } // end detail
  
  //! Generic method to split a string a statically defined number of times
  //! at a provided delimeter. If the delimiter occurs less than N times,
  //! empty values are returned. If the delimiter occurs more than N times,
  //! the last entry will contain the remainder of the string after the 
  //! (N-1)th delimiter. If the delimiter occurs in the provided string <=N times,
  //! the delimiter will not be present within the output values.
  //! @tparam N The number of delimiters to use when parsing.
  //! @param begin An iterator to the beginning of the input string.
  //! @param end An iterator to the end of the input string.
  //! @param delimiter The character sequence to split on.
  //! @return A tuple with N types, where <0> contains the value provided
  //!   within the source range that preceeds the first delimiter.
  //! @note gcc and clang behave differently with regard to the order in
  //!   which they iterate through tuple indices. It is neccessary to reverse
  //!   the sequence of the calculated result to ensure the indexes are 
  //!   logical when using a GNU compiler.
  template <size_t N>
  typename detail::generate_tuple_type<std::string, N>::type split(
    std::string::iterator begin, 
    std::string::iterator end, 
    std::string delimiter)      
  {
    return 
    
      // When using GNU is it necessary to reverse the indices.
      #if defined(__GNUC__) || defined(__GNUG__)
        detail::invert(
      #endif
      
      detail::generate_tuple<N>(
        [&begin, &end, &delimiter](int i) -> std::string
        {
          std::string temp(begin, end);

          #if defined(__GNUC__) || defined(__GNUG__)
          if (i == 0)
          #else
          if (i == (N - 1))
          #endif
          {
            return temp;
          }
          else
          {
            auto iter_of_first = temp.find_first_of(delimiter);

            if (iter_of_first != std::string::npos)
            {
              temp = temp.substr(0, iter_of_first);
              begin += (iter_of_first + delimiter.size());

              return temp;
            }
            else
            {
              return std::string();
            }
          }
        })
        
        #if defined(__GNUC__) || defined(__GNUG__)
        )
        #endif   
        
        ; // Intentional
        
  } // end split

} // end tjh

#endif // SPLIT_H
