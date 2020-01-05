//
// Created by tomer on 24/12/2019.
//

#include "SymbolTable.h"
#include <iterator>
#include <list>
#include <sstream>


SymbolTable::~SymbolTable() {
  /**
   * deleting th symbol table, all vars in it should be removed from the all vars list
   */
  for (const pair<const string, list<Var*>::iterator>& p: *this->scope_vars) {
    this->all_vars_mutex->lock();
    delete(*p.second);
    this->all_vars->erase(p.second);
    this->all_vars_mutex->unlock();
  }
  delete this->scope_vars;
}

void SymbolTable::set(string name, double new_val) {
  /**
   * change the value of a var
   */
   if (this->scope_vars->find(name) == this->scope_vars->end()) {
     // no such var in this scope
     if (this->outer_scope != nullptr) {
       // look in outer scope
       this->outer_scope->set(name, new_val);
       return;
     }
     throw "no such var";
   }
   this->all_vars_mutex->lock();
  (*(*this->scope_vars)[name])->set_value(new_val);
  this->all_vars_mutex->unlock();
}

double SymbolTable::get(string name) {
  /**
   * get the value of a var
   */
  if (this->scope_vars->find(name) == this->scope_vars->end()) {
    // no such var in this scope
    if (this->outer_scope != nullptr) {
      // look in outer scope
      return this->outer_scope->get(name);
    }
    throw "no such var";
  }
  this->all_vars_mutex->lock();
  double to_return = (*(*this->scope_vars)[name])->get_value();
  this->all_vars_mutex->unlock();
  return to_return;
}

void SymbolTable::add(Var* v) {
  /**
   * add var to the scope, and to all vars
   */
   this->all_vars_mutex->lock();
   this->all_vars->push_front(v);
   (*this->scope_vars)[v->get_name()] = this->all_vars->begin();
   this->all_vars_mutex->unlock();
}

Interpreter SymbolTable::getInterpreter() {
  /**
   * crete interpreter from with the vars in the symbol table
   */
  Interpreter interpreter;
  if (this->outer_scope != nullptr)
    // if there is an outer scope, add its vars first.
    interpreter = this->outer_scope->getInterpreter();
  for (pair<string, list<Var*>::iterator> p: *this->scope_vars) {
    interpreter.setVariables(p.first + "=" + to_string((*p.second)->get_value()));
  }
  return interpreter;
}