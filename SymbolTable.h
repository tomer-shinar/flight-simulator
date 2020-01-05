//
// Created by tomer on 24/12/2019.
//

#ifndef FLIGHT_SIMULATOR__SYMBOLTABLE_H_
#define FLIGHT_SIMULATOR__SYMBOLTABLE_H_

#include <unordered_map>
#include "Var.h"
#include <iterator>
#include <mutex>
#include <list>
#include "Interpreter.h"

class SymbolTable {
  /**
   * class to hold the symbol table for scope.
   * implemented by unordered map from name to iterator<Var>
   */
 private:
  unordered_map<string, list<Var*>::iterator>* scope_vars;
  list<Var*>* all_vars;
  mutex* all_vars_mutex;
  SymbolTable* outer_scope;
 public:
  SymbolTable(list<Var*>* all_vars, mutex* m, SymbolTable* outer): all_vars(all_vars), all_vars_mutex(m), outer_scope(outer) {
    this->scope_vars = new unordered_map<string, list<Var*>::iterator>();
  }
  SymbolTable(list<Var*>* all_vars, mutex* m): SymbolTable(all_vars, m, nullptr) {};
  ~SymbolTable();
  void set(string name, double new_val);
  double get(string name);
  void add(Var* v);
  Interpreter getInterpreter();
};

#endif //FLIGHT_SIMULATOR__SYMBOLTABLE_H_
