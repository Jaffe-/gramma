#include "production.h"
#include "grammar.h"
#include <string>

std::string gen()
{
  static int n = 0;
  return "GEN" + std::to_string(n++);
}

using namespace std;
int main()
{

  Grammar<string> G = {"rexpr", {
    {"rexpr", {
	{"rexpr", "+", "rterm"},
	{"rterm"}}},
    {"rterm", {
	{"rterm", "rfactor"},
	{"rfactor"}}},
    {"rfactor", {
	{"rfactor", "*"},
	{"rprimary"}}},
    {"rprimary", {
	{"a"}, {"b"}}}
    }};
   
  Grammar<string> H = {"S", {
      {"S", {
	  {"E"},
	  {"S", "A"},
	  {"A"}}},
      {"E", {
	  {"if", "(", "C", ")", "{", "S", "}"},
	  {"if", "(", "C", ")", "{", "S", "}", "else", "{", "S", "}"}}},
      {"A", {
	  {"V", ":=", "T"}}},
      {"T", {
	  {"a"},
	  {"b"}}},
      {"V", {
	  {"x"},
	  {"y"}}},
      {"C", {
	  {"V", "O", "T"},
	  {"T", "O", "V"},
	  {"V", "O", "V"},
	  {"T", "O", "T"}}},
      {"O", {
	  {"<"},
	  {">"}}}}
  };

  Grammar<string> G2 = {"A", {
      {"A", {
	  {"g", "B"},
	  {"g", "C", "D", "E"}}},
      {"B", {
	  {"a"},
	  {"b", "C"}}},
      {"C", {
	  {"c"},
	  {""}}},
      {"D", {
	  {"D", "d"},
	  {"d"}}},
      {"E", {
	  {"E", "e"},
	  {"f"}}}
    }
  };

  std::cout << G2;
  auto f = G2.leftFactor(&gen);
  std::cout << f;
  auto f2 = f.eliminate_imm_left_recursion(&gen);
  std::cout << f2;

  for (auto& s : f2.first()) {
    std::cout << "FIRST(" << s.first << "): " << s.second << std::endl;
  }

  for (auto& s : f2.follow("$")) {
    std::cout << "FOLLOW(" << s.first << "): " << s.second << std::endl;
  }
  
}
