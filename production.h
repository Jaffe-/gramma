#pragma once

#include "functional.h"
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <functional>

template <typename T>
class Production;

template <typename T>
std::ostream& operator<<(std::ostream&, Production<T>&);

template <typename T>
class Production {
private:
  T lhs;
  std::vector<T> rhs;
  
public:
  Production(T lhs, std::vector<T> rhs) : lhs{lhs}, rhs{rhs} {};

  T getlhs() const { return lhs; };
  const std::vector<T>& getrhs() const { return rhs; };

  void substitute(Production replacement);
  void remove_first(int n);
  std::vector<T> substring(int from, int to);
  std::vector<T> drop(int n);

  bool is_empty() const;

  bool is_imm_left_recursive() const;

  void append(T symbol);

  bool has(T symbol);

  std::vector<T> trail(T symbol);

  friend std::ostream& operator<< <T>(std::ostream&, Production&);

};

template<typename T>
void Production<T>::substitute(Production<T> replacement) 
{
  auto pos = std::find_if(rhs.begin(), rhs.end(),
			  [&replacement](auto& e) {
			    return e == replacement.lhs;
			  });
  if (pos != rhs.end()) {
    auto newpos = rhs.erase(pos);
    for (auto& term : replacement.rhs) {
      rhs.insert(newpos++, term);
    }
  }
}

template <typename T>
void Production<T>::remove_first(int n)
{
  rhs.erase(std::begin(rhs), std::begin(rhs) + n);
}

template <typename T>
std::vector<T> Production<T>::substring(int from, int to)
{
  std::vector<T> substr;
  std::copy(rhs.begin() + from, rhs.begin() + to, std::back_inserter(substr));
  return substr;
}

template <typename T>
std::vector<T> Production<T>::drop(int n)
{
  auto sub = substring(n, rhs.size());
  if (sub.size() == 0)
    sub.push_back(T());
  return sub;
}

template <typename T>
bool Production<T>::is_empty() const
{
  return rhs.size() == 1 && rhs[0] == T();
}

template <typename T>
void Production<T>::append(T symbol)
{
  rhs.push_back(symbol);
}

template <typename T>
std::vector<T> Production<T>::trail(T symbol)
{
  std::vector<T> result;
  auto it = std::find(rhs.begin(), rhs.end(), symbol);
  if (std::next(it) != rhs.end())
    std::copy(it+1, rhs.end(), std::back_inserter(result));
  return result;
}


template<typename T>
std::ostream& operator<<(std::ostream& o, std::vector<T>& v)
{
  if (v.size() == 1 && v[0] == T())
    o << "<e> ";
  for (auto& sym : v)
    o << sym << " ";
  return o;
}

template <typename T>
bool Production<T>::has(T symbol)
{
  return std::find(rhs.begin(), rhs.end(), symbol) != rhs.end();
}

template<typename T>
std::ostream& operator<<(std::ostream& o, Production<T>& p)
{
  o << p.lhs << " -> ";
  o << p.rhs;
  
  return o << std::endl;
}

template<typename T>
bool Production<T>::is_imm_left_recursive() const
{
  return rhs.size() > 0 && lhs == rhs[0];
}
