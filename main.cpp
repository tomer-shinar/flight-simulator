//
// Created by tomer on 24/12/2019.
//
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "Interpreter.h"
#include "SymbolTable.h"
#include "commands.h"

#define VAR_COMMAND "var"
#define SERVER_COMMAND "openDataServer("
#define CLIENT_COMMAND "connectControlClient("
#define WHILE_COMMAND "while"
#define IF_COMMAND "if"
#define PRINT_COMMAND "Print("
#define SLEEP_COMMAND "Sleep("

using namespace std;

struct DefinedFunc {
  list<Command*> commands;
  string varName;
};

list<string> lexer(ifstream &is) {
  /**
   * read the file to list of lines
   */
  list<string> lines;
  string line;
  while (getline(is, line)) {
    lines.push_back(strip(line));
  }
  return lines;
}

list<Command*> parser(list<string>::iterator &it, list<string>::iterator end, bool* to_continue,
    map<string, DefinedFunc> funcs) {
  /**
   * parse the lines to commands
   */
   list<Command*> commands;
   for (; it != end; it++) {
     if (*it == "}")
       //end of inner block
       return commands;
     if (it->find(VAR_COMMAND) == 0) {
       //start with var
       commands.push_back(new DefineVarCommand(it->substr(strlen(VAR_COMMAND))));
     } else if (it->find(SERVER_COMMAND) == 0) {
       commands.push_back(new OpenServerCommand(it->substr(strlen(SERVER_COMMAND), it->size() -
       strlen(SERVER_COMMAND) - 1), to_continue));
     } else if (it->find(CLIENT_COMMAND) == 0) {
       commands.push_back(new ConnectCommand(it->substr(strlen(CLIENT_COMMAND), it->find(",") -
       strlen(CLIENT_COMMAND)), it->substr(it->find(",") + 1, it->length() - 2 - it->find(",")),
           to_continue));
     } else if (it->find(WHILE_COMMAND) == 0) {
       string condition = it->substr(strlen(WHILE_COMMAND), it->length() - 1 - strlen(WHILE_COMMAND));
       commands.push_back(new WhileCommand(condition, parser(++it, end, to_continue, funcs)));
     } else if (it->find(IF_COMMAND) == 0) {
       string condition = it->substr(strlen(IF_COMMAND), it->length() - 1 - strlen(IF_COMMAND));
       commands.push_back(new IfCommand(condition, parser(++it, end, to_continue, funcs)));
     } else if (it->find(PRINT_COMMAND) == 0) {
       commands.push_back(new PrintCommand(it->substr(strlen(PRINT_COMMAND), it->length() - 1 - strlen(PRINT_COMMAND))));
     } else if (it->find(SLEEP_COMMAND) == 0) {
       commands.push_back(new SleepCommand(it->substr(strlen(SLEEP_COMMAND), it->length() - 1 - strlen(SLEEP_COMMAND))));
     } else if (it->find("=") != string::npos) {
       commands.push_back(new AssignVarCommand(it->substr(0, it->find("=")),
           it->substr(it->find("=") + 1, it->length() - it->find("="))));
     } else if (it->find("var") != string::npos) {
       //func definition
       string funcName = strip(it->substr(0, it->find("(") - 1));
       string value = strip(it->substr(it->find("var") + strlen("var"), it->find(")") - it->find("var") - strlen("var")));
       list<Command*> blockCommands = parser(++it, end, to_continue, funcs);
       funcs.insert(pair<string, DefinedFunc>(funcName, DefinedFunc{blockCommands, value}));
     } else if(it->find("(") != string::npos && funcs.find(it->substr(0, it->find("(") -1)) != funcs.end()) {
       //this is a func call
       DefinedFunc f = funcs[it->substr(0, it->find("(") -1)];
       commands.push_back(new FuncCommand(f.commands, f.varName, it->substr(it->find("(") + 1,
           it->find(")") - it->find("(") - 1)));
     }
     else if (!it->empty()) {
       throw "illegal command";
     }
   }
   return commands;
}

int main(int argc, char *argv[]) {
  //lexer
  ifstream is;
  is.open(argv[1]);
  list<string> lines = lexer(is);
  is.close();
  //parsing
  bool toContinue = true;
  auto start = lines.begin();
  map<string, DefinedFunc> funcs;
  list<Command*> commands = parser(start, lines.end(), &toContinue, funcs);
  //run commands
  list<Var*> vars;
  mutex mutex;
  SymbolTable *s = new SymbolTable(&vars, &mutex);
  for (Command *c: commands) {
    c->execute(s, &mutex, &vars);
  }
  toContinue = false;
  return 0;
}
