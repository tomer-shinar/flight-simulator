//
// Created by tomer on 24/12/2019.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "commands.h"
#include "Var.h"
#include <thread>
#include <fstream>
#include <arpa/inet.h>

using namespace std;

vector<string> getVarsNames() {
  /**
   * get all the simulator vars names in the order of the xml
   */
  ifstream f;
  f.open("generic_small.xml");
  string line;
  vector<string> names;
  while (getline(f, line)) {
    if (line.find("<node>") != string::npos) {
      //there is node in this line
      names.push_back(line.substr(line.find("<node>") + strlen("<node>"),
          line.find("</node>") - line.find("<node>") - strlen("<nod>") - 1));
    }
  }
  f.close();
  return names;
}
vector<double> getValues(string buffer) {
  /**
   * takes a string of doubles separated by spaces, and return vector of doubles
   */
   vector<double> values;
   string token;
   istringstream is(buffer);
   while(getline(is, token, ',')) {
     if (token == "\n")
       break;
     values.push_back(strtod(token.c_str(), nullptr));
   }
   return values;
}
unordered_map<string, double> getMap(vector<string> names, vector<double> values) {
  /**
   * takes the names and vakues and put in map
   */
  unordered_map<string, double> mp;
  for (int i = 0; i < names.size(); i++) {
    mp.insert(pair<string, double>(names[i], values[i]));
  }
  return mp;
}
void updateVars(unordered_map<string, double> newVals, mutex* mutex, list<Var*>* vars) {
  /**
   * updates each of the vars that takes values from simulator
   */
   mutex->lock();
   for(auto it = vars->begin(); it != vars->end(); ++it) {
     auto* rv = dynamic_cast<ReadingVar*>(&(**it));
     if (rv != nullptr) {
       (*it)->set_value(newVals[rv->getSource()]);
     }
   }
   mutex->unlock();
}
void runServer(SymbolTable *s, mutex *mutex, list<Var*> *vars, int client_socket, bool* to_continue) {
  /**
   * run the server that read values from the server and updates the vars.
   * to_continue is true while the scope of the server is running.
   */
  char buffer[1024] = {0};
  int write_from = 0;
  while (*to_continue) {
    write_from += read(client_socket, buffer, 1024);
    vector<string> varsNames = getVarsNames();
    string mess(buffer);
    if (mess.find("\n") == string::npos) {
      continue;
    }
    vector<double> values = getValues(mess.substr(0, mess.find("\n")));
    strcpy(buffer, mess.substr(mess.find("\n")).c_str() + 1);
    write_from -= mess.find("\n");
    unordered_map<string, double> newValues = getMap(varsNames, values);
    updateVars(newValues, mutex, vars);
  }
  close(client_socket);
}
void OpenServerCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   *open server, wait for client to connect and run another thread.
   * the other thread read values from the client and assign to vars that should receive those values
   */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  Expression* port_exp = s->getInterpreter().interpret(this->port_str);
  address.sin_port = htons(port_exp->calculate()); //calculating port number
  delete(port_exp);
  if (bind(sockfd, (struct sockaddr *) &address, sizeof(address)) == -1) {
    throw "failed to bind";
  }
  if (listen(sockfd, 1) == -1)
    throw "failed listen";
  int client_socket = accept(sockfd, (struct sockaddr *) &address, (socklen_t*) &address);
  if (client_socket == -1)
    throw "failed accepting";
  close(sockfd);
  thread t(runServer, s, mutex, vars, client_socket, this->to_continue);
  t.detach();
}

void runClient(SymbolTable *s, mutex *mutex, list<Var*> *vars, int client_socket, bool* to_continue) {
  /**
   * run the client thread that set values in the simulator according to the vars
   */
   while(*to_continue) {
     mutex->lock();
     for (auto it = vars->begin(); it != vars->end(); ++it) {
       auto *wv = dynamic_cast<WritingVar *>(&(**it));
       if (wv != nullptr && wv->newData()) {
         string mess = "set " + wv->getTarget().substr(1) + " " + to_string(wv->get_value()) + " \n";
         int is_sent = send(client_socket, mess.c_str(), strlen(mess.c_str()), 0);
         if (is_sent == -1) {
           throw "failed to send";
         }
         wv->dataRead();
       }
     }
     mutex->unlock();
   }
   close(client_socket);
}
void ConnectCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * connect to the server as client.
   * create a thread that change the values when the vars that changes the simulator are changing
   */
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(this->ip.substr(1, this->ip.length() - 2).c_str());
  Expression* port_exp = s->getInterpreter().interpret(this->port_str);
  address.sin_port = htons(port_exp->calculate()); //calculating port number
  delete(port_exp);
  int is_connect = connect(client_socket, (struct sockaddr*)&address, sizeof(address));
  if (is_connect == -1)
    throw "failed to connect";
  thread t(runClient, s, mutex, vars, client_socket, this->to_continue);
  t.detach();
}

DefineVarCommand::DefineVarCommand(const string& line) {
  int name_end, param_start;
  if (line.find("->") != string::npos) {
    type = 0;
    name_end = line.find("->");
    param_start = name_end + 2;
  } else if (line.find("<-") != string::npos) {
    type = 1;
    name_end = line.find("<-");
    param_start = name_end + 2;
  } else if (line.find('=') != string::npos) {
    type = 3;
    name_end = line.find('=');
    param_start = name_end + 1;
  } else {
    cout << line << endl;
    throw "illegal var declaration";
  }
  this->name = strip(line.substr(0, name_end));
  this->param = strip(line.substr(param_start));
}
void DefineVarCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * define a var and add to scope
   */
   if (this->type == 0) {
     s->add(new WritingVar(this->name, this->param.substr(strlen("sim(") + 1,
         this->param.length() - 3 - strlen("sim("))));
   } else if (this->type == 1) {
     s->add(new ReadingVar(this->name, this->param.substr(strlen("sim(") + 1,
         this->param.length() - 3 - strlen("sim("))));
   } else {
     //regular var
     Expression* toAssign = s->getInterpreter().interpret(param);
     s->add(new Var(this->name, toAssign->calculate()));
     delete(toAssign);
   }
}

void AssignVarCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * handle var assignment
   */
  Expression* toAssign = s->getInterpreter().interpret(this->value);
  s->set(strip(this->varName), toAssign->calculate());
  delete(toAssign);
}

void ConditionCommand::buildSymbolTable(SymbolTable* s, mutex* mutex, list<Var*>* vars) {
  this->innerScope = new SymbolTable(vars, mutex, s);
}
void ConditionCommand::deleteSymboleTable() {
  delete(this->innerScope);
}
bool ConditionCommand::condition() {
  /**
   * check the condition
   */
  Expression* conditionExpression = this->innerScope->getInterpreter().interpret(this->conditionString);
  bool result = conditionExpression->calculate();
  delete(conditionExpression);
  return result;
}
void ConditionCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * run all commands in block once
   */
   for (Command *c: this->commands) {
     c->execute(this->innerScope, mutex, vars);
   }
}

void WhileCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * while condition is true, run the inner block
   */
   buildSymbolTable(s, mutex, vars);
   while (this->condition()) {
     ConditionCommand::execute(s, mutex, vars);
   }
   this->deleteSymboleTable();
}

void IfCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * if condition is true, run the inner block
   */
  buildSymbolTable(s, mutex, vars);
  if (this->condition()) {
    ConditionCommand::execute(s, mutex, vars);
  }
  this->deleteSymboleTable();
}

void PrintCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  /**
   * print the param
   */
   if (this->toPrint.find('"') != string::npos) {
     //string
     cout << this->toPrint.substr(this->toPrint.find('"') + 1, this->toPrint.length() - 2 -
     this->toPrint.find('"')) << endl;
   } else {
     //expression
     Expression* exp = s->getInterpreter().interpret(toPrint);
     cout << exp->calculate() << endl;
     delete(exp);
   }
}

void SleepCommand::execute(SymbolTable *s, mutex *mutex, list<Var*> *vars) {
  // sleep
  Expression* exp = s->getInterpreter().interpret(this->timeExpression);
  this_thread::sleep_for(chrono::milliseconds((int) exp->calculate()));
  delete(exp);
}

void FuncCommand::execute(SymbolTable *s, mutex *mutex, list<Var *> *vars) {
  /**
   * run the function
   */
   //create scope to the func with the param in it
   SymbolTable* funcScope = new SymbolTable(vars, mutex, s);
   Expression* exp = s->getInterpreter().interpret(this->valueExpression);
   funcScope->add(new Var(this->paramName, exp->calculate()));
   delete(exp);
   //run the block
   for (Command* c: this->commands) {
     c->execute(funcScope, mutex, vars);
   }
   //delete the scope
   delete(funcScope);
}

string strip(string s) {
  while (s[0] == '\t' || s[0] == ' ') {
    //starts with tab
    s = s.substr(1);
  }
  while (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\n') {
    //starts with tab
    s = s.substr(0, s.size() - 1);
  }
  return s;
}