#pragma once

#include "production.h"
#include "functional.h"
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <functional>
#include <utility>


template <typename T>
using Rule = std::pair<T, std::vector<Production<T>>>;

template <typename T>
class Grammar;

template <typename T>
std::ostream& operator<< (std::ostream&, Grammar<T>&);

template <typename T>
class Grammar {
private:
  T start_symbol;
  std::set<T> nonterminals;
  std::set<T> terminals;
  std::set<T> symbols;
  std::map<T, std::vector<Production<T>>> rules;

  T add_nonterminal(T);
  bool join_to(std::set<T>& dst, std::set<T>& src);

public:
  Grammar(T start_symbol, std::initializer_list<std::pair<T, std::vector<std::vector<T>>>> rule_list);

  const std::set<T>& get_nonterminals() const { return nonterminals; };
  const std::set<T>& get_terminals() const { return terminals; };
  const std::set<T>& get_symbols() const { return symbols; };
  
  bool is_imm_left_recursive() const;
  bool is_imm_left_recursive(const Rule<T>& rule) const;

  friend std::ostream& operator<< <T>(std::ostream&, Grammar&);

  template <typename F>
  Grammar leftFactor(F);

  template <typename F>
  Grammar<T> eliminate_imm_left_recursion(F generator);

  bool has_empty_production(T nonterminal);
  bool is_terminal(T symbol);
  bool is_nonterminal(T nt);
  
  std::set<T> first(T symbol);
  std::map<T, std::set<T>> first();
  std::set<T> first(std::vector<T>);
  
  std::map<T, std::set<T>> follow(T terminator);
};

template <typename T>
Grammar<T>::Grammar(T start_symbol, std::initializer_list<std::pair<T, std::vector<std::vector<T>>>> rule_list)
  : start_symbol(start_symbol)
{

  /* First pass, adding nonterminals and productions */
  for (auto& rule : rule_list) {
    T nonterminal = rule.first;
    nonterminals.insert(nonterminal);
    symbols.insert(nonterminal);

    for (auto& rhs : rule.second) {
      rules[nonterminal].push_back(Production<T>(nonterminal, rhs));
      for (auto& sym : rhs)
	symbols.insert(sym);
    }
  }
  
  for (auto& s : symbols) {
    if (!is_nonterminal(s))
      terminals.insert(s);
  }
}

/* 
   Check if a given rule in the grammar is left recursive 
*/
template <typename T>
bool Grammar<T>::is_imm_left_recursive(const Rule<T>& rule) const
{
  return std::any_of(rule.second.begin(), rule.second.end(), std::mem_fn(&Production<T>::is_imm_left_recursive));
}


/* 
   Check if the grammar is left recursive
*/
template <typename T>
bool Grammar<T>::is_imm_left_recursive() const
{
  return std::any_of(rules.begin(), rules.end(), &is_imm_left_recursive);
}


template <typename T>
std::ostream& operator<<(std::ostream& o, Grammar<T>& g)
{
  o << "Terminals:\t";

  for (auto& t : g.terminals)
    o << t << " ";
  
  o << std::endl << "Nonterminals:\t";

  for (auto& nt : g.nonterminals)
    o << nt << " ";
  
  o << std::endl << "Rules: " << std::endl;
  for (auto& rule : g.rules) {
    o << rule.first << "\t -> \t";
    for (auto it = rule.second.begin(); it != rule.second.end(); it++) {
      auto rhs = it->getrhs();
      o << rhs;
      if (std::next(it) != rule.second.end())
	o << "| ";
    }
    o << std::endl;
  }
  return o;
}


/* 
   Left factor the grammar
*/
template <typename T>
template <typename F>
Grammar<T> Grammar<T>::leftFactor(F generator)
{
  Grammar<T> result = *this;

  for (auto& rule : result.rules) {
    auto& prods = rule.second;
    if (prods.size() > 1) {
      std::vector<T> common_string;

      // Get a vector of the lengths of each production
      auto length_vec = mapv(prods, [](auto x) { return x.getrhs().size(); });
      
      for (unsigned int i = 0; i < *std::min_element(std::begin(length_vec), std::end(length_vec)); i++) {
	T sym = prods[0].getrhs()[i];

	if (std::all_of(prods.begin(), prods.end(), [i, &sym](auto x) { return x.getrhs()[i] == sym; })) 
	  common_string.push_back(sym);
	else
	  break;
      }
      
      if (common_string.size() > 0) {
	T new_nonterminal = result.add_nonterminal(generator());
	
	// Add productions to new non-terminal
	for (auto& prod : prods) {
	  result.rules[new_nonterminal].push_back(
	    Production<T> {new_nonterminal, prod.drop(common_string.size()) });
	}
	
       	auto new_production = common_string;
	new_production.push_back(new_nonterminal);
	prods.erase(prods.begin(), prods.end());
	prods.push_back(Production<T>(rule.first, new_production));
	
      }
    }
  }
  
  return result;
}


/*
  Eliminate immediate left recursion. 

  generator is a supplied function to generate new symbols for the new nonterminals 
  generated by the algorithm.
*/
template <typename T>
template <typename F>
Grammar<T> Grammar<T>::eliminate_imm_left_recursion(F generator)
{
  Grammar<T> result = *this;
  
  for (auto& rule : result.rules) {
    if (is_imm_left_recursive(rule)) {

      T new_nt = result.add_nonterminal(generator());
      
      for (auto& p : filterv(rule.second, std::mem_fn(&Production<T>::is_imm_left_recursive))) {
	if (!p.is_empty()) {
	  auto rhs = p.drop(1);
	  rhs.push_back(new_nt);
	  result.rules[new_nt].push_back(Production<T> {new_nt, rhs});
	}
      }
      result.rules[new_nt].push_back(Production<T> {new_nt, {""}});
      
      rule.second.erase(std::remove_if(rule.second.begin(), rule.second.end(),
				       std::mem_fn(&Production<T>::is_imm_left_recursive)));
      
      for (auto& prod : rule.second) 
	prod.append(new_nt);
    }
  }

  return result;
}


/* 
   Find the first set of a given symbol in te grammar
*/
template <typename T>
std::set<T> Grammar<T>::first(T symbol)
{
  std::set<T> result;

  if (symbol == T())
    return result;
  
  if (is_terminal(symbol)) 
    result.insert(symbol);

  else {
    if (has_empty_production(symbol)) 
      result.insert(T());

    if (rules.find(symbol) != rules.end()) {
      auto& prods = rules[symbol];
      
      for (auto& prod : prods) {
	for (auto& sym : prod.getrhs()) {
	  auto first_set = first(sym);
	  result.insert(first_set.begin(), first_set.end());
	  
	  // Stop if the current symbol has no empty symbol in FIRST
	  if (!has(first_set, T()))
	    break;
	} 
      }
    }
    else
      std::cout << "ERROR: " << symbol << " is not in grammar." << std::endl;
  }
  return result;
}


/*
  Find the first set of a string
*/
template <typename T>
std::set<T> Grammar<T>::first(std::vector<T> string)
{
  std::set<T> result;
  for (auto& sym : string) {
    auto first_set = first(sym);
    std::copy_if(first_set.begin(), first_set.end(), std::inserter(result, result.begin()),
		 [] (auto s) { return s != T(); });

    if (first_set.find(sym) == first_set.end())
      break;
  }

  return result;
}


/*
  Compute all first sets of the grammar
*/
template <typename T>
std::map<T, std::set<T>> Grammar<T>::first()
{
  std::map<T, std::set<T>> first_sets;
  
  for (auto& s : symbols) {
    if (s != T())
      first_sets[s] = first(s);
  }

  return first_sets;
}


/*
  Compute all follow sets of the grammar's symbols

  terminator is the symbol designating end of string
 */
template <typename T>
std::map<T, std::set<T>> Grammar<T>::follow(T terminator)
{
  std::map<T, std::set<T>> follow_sets;
  follow_sets[start_symbol].insert(terminator);

  bool changed = true;

  while (changed) {
    changed = false;
    for (auto& rule : rules) {
      for (auto& prod : rule.second) {
	for (auto& nt : nonterminals) {
	  if (prod.has(nt)) {
	    auto trail = prod.trail(nt);

	    if (trail.size() > 0) {
	      for (auto& e : first(trail)) {
		if (e == T()) {
		  if (join_to(follow_sets[nt], follow_sets[prod.getlhs()]))
		    changed++;
		} else {
		  auto ret = follow_sets[nt].insert(e);
		  if (ret.second == true)
		    changed = true;
		}
	      }
	    }
	      
	    // Case A => aB
	    else {
	      if (join_to(follow_sets[nt], follow_sets[prod.getlhs()]))
		changed = true;
	    }
	  }
	}
      }
    }
  }

  return follow_sets;
  
}



/* 

   Utility functions 

*/

template <typename T>
T Grammar<T>::add_nonterminal(T symbol)
{
  nonterminals.insert(symbol);
  symbols.insert(symbol);
  return symbol;
}

template <typename T>
bool Grammar<T>::has_empty_production(T nonterminal)
{
  if (rules.find(nonterminal) != rules.end()) {
    auto& prods = rules[nonterminal];
    return std::any_of(prods.begin(), prods.end(), std::mem_fn(&Production<T>::is_empty));
  }
  return false;
}

template <typename T>
bool Grammar<T>::is_terminal(T symbol)
{
  return has(terminals, symbol);
}

template <typename T>
bool Grammar<T>::is_nonterminal(T nt)
{
  return has(nonterminals, nt);
}

template <typename T>
std::ostream& operator<< (std::ostream& o, std::set<T> s)
{
  for (auto& e : s) {
    if (e == T())
      o << "<e>" << " ";
    else
      o << e << " ";
  }
  return o;
}

template <typename T>
bool Grammar<T>::join_to(std::set<T>& dst, std::set<T>& src)
{
  bool joined = false;
  for (auto& e : src) {
    auto ret = dst.insert(e);
    if (ret.second == true)
      joined = true;
  }
  return joined;
}


