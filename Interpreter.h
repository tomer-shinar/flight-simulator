//
// Created by tomer on 16.11.2019.
//

#ifndef EX1__INTERPRETER_H_
#define EX1__INTERPRETER_H_

#include "ex1.h"
#include "Expression.h"
#include <map>

class Interpreter {
  std::map<string, double> mp;
 public:
  Interpreter() {};
  void setVariables(const string& vars);
  Expression *interpret(string infix);
  ~Interpreter() {}
};

#endif //EX1__INTERPRETER_H_
