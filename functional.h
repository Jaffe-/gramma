#pragma once

#include <vector>
#include <algorithm>

/* 

   Convenience functions for functional programming

*/

template <typename C, typename T>
bool has(C collection, T val)
{
  return std::find(std::begin(collection), std::end(collection), val) != std::end(collection);
}

template <typename T, typename F>
auto filterv(const std::vector<T>& v, F pred)
{
  std::vector<T> result;    
  std::copy_if(v.begin(), v.end(), std::back_inserter(result), pred);
  return result;
}

template <typename T, typename F>
auto mapv(const std::vector<T>& v, F f)
{
  // This monstrosity declares the value type of result to be of f's return type
  std::vector<decltype(f(std::declval<T>()))> result (v.size());

  std::transform(v.begin(), v.end(), result.begin(), f);
  return result;
}
