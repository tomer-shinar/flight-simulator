//
// Created by tomer on 24/12/2019.
//

#ifndef FLIGHT_SIMULATOR__COMMANDS_H_
#define FLIGHT_SIMULATOR__COMMANDS_H_

#include <mutex>
#include <unordered_map>
#include <iterator>
#include <list>
#include <utility>
#include "Var.h"
#include "SymbolTable.h"

using namespace std;

class Command {
 public:
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars) = 0;
  ~Command() = default;
};

class OpenServerCommand: public Command {
 private:
  string port_str;
  bool* to_continue;
 public:
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
  OpenServerCommand(string s, bool* to_continue) : port_str(std::move(s)), to_continue(to_continue) {};
};

class ConnectCommand: public Command {
 private:
  string ip;
  string port_str;
  bool* to_continue;
 public:
  ConnectCommand(string ip, string port_str, bool* to_continue): ip(std::move(ip)), port_str(std::move(port_str)),
  to_continue(to_continue) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class DefineVarCommand: public Command {
 private:
  int type;
  string name;
  string param;
 public:
  DefineVarCommand(const string& line);
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class AssignVarCommand: public Command {
 private:
  string varName;
  string value;
 public:
  AssignVarCommand(string name, string val): varName(std::move(name)), value(std::move(val)) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class ConditionCommand: public Command {
 private:
  SymbolTable* innerScope = nullptr;
  string conditionString;
  list<Command*> commands;
 protected:
  void buildSymbolTable(SymbolTable* s, mutex* mutex, list<Var*>* vars);
  void deleteSymboleTable();
  bool condition();
 public:
  ConditionCommand(string condition, list<Command*> com): conditionString(std::move(condition)), commands(std::move(com)) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class WhileCommand: public ConditionCommand {
 public:
  WhileCommand(string condition, list<Command*> commands): ConditionCommand(std::move(condition), std::move(commands)) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class IfCommand: public ConditionCommand {
 public:
  IfCommand(string condition, list<Command*> commands): ConditionCommand(std::move(condition), std::move(commands)) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class PrintCommand: public Command {
 private:
  string toPrint;
 public:
  PrintCommand(string param): toPrint(std::move(param)) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};

class SleepCommand: public Command {
 private:
  string timeExpression;
 public:
  SleepCommand(string param): timeExpression(std::move(param)) {};
  virtual void execute(SymbolTable* s, mutex* mutex, list<Var*>* vars);
};


string strip(string s);

#endif //FLIGHT_SIMULATOR__COMMANDS_H_
