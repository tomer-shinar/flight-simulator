//
// Created by tomer on 16.11.2019.
//

#include "Interpreter.h"
#include <stack>
#include <iostream>

string var_name("[a-zA-Z][a-zA-Z0-9_]*");
string number("\\d+\\.?\\d*");
string oper("([/+\\-/*//]|==|!=|<=|<|>=|>)");

enum Operator { plus, minus, mul, div1, Uplus, Uminus, bracket, equals, not_equal, greater, greater_equal, lower,
        lower_equal}; // representing operators that go to the stack
int priority[] = {2, 2, 3, 3, 4, 4, 0, 1, 1, 1, 1, 1, 1}; //each Operator have a priority according to priority[operator]

void push_operator(stack<Operator> &op_stack, stack<Expression*> &output, string op, string prev, sregex_iterator &next);
void handle_var(stack<Expression*> &output, string var, string prev, map<string, double> mp);
void handle_number(stack<Expression*> &output, string token);
void handle_closing_bracket(stack<Operator> &op_stack, stack<Expression*> &output);
void handle_opening_bracket(stack<Operator> &op_stack, string prev);
void pop_operator(stack<Operator> &op_stack, stack<Expression*> &output);

void Interpreter::setVariables(const string &vars) {
  //split vars to tokens:
  string token;
  std::istringstream tokenStream(vars);
  while (std::getline(tokenStream, token, ';')) {
    //extract the var name and value from the token

    int i = token.find('=');
    if (i == -1)
      //there is no "=" in the token"
      throw "wrong syntax";

    string name = token.substr(0, i);
    if (!regex_match(name.begin(), name.end(), regex(var_name)))
      //the var name is illegal
      throw "wrong syntax";

    char *end;
    double val = strtod(token.substr(i + 1).c_str(), &end);
    if ((strcmp(end, "") != 0) || (strcmp(token.substr(i + 1).c_str(), "") == 0))
      //not a number
      throw "wrong syntax";

    this->mp[name] = val;
  }

}
Expression *Interpreter::interpret(string infix) {
  //regular expression representing each token. token can be positive number, var name operator or (, )
  string token_str("(" + var_name + "|" + number + "|" + oper + "|[/(/)]| )");
  regex token_regex(token_str);
  if (!regex_match(infix.begin(), infix.end(), regex(token_str + "+")))
    //can't split the infix to legal tokens
    throw "wrong syntax";

  stack<Operator> op_stack; // operator stack
  stack<Expression*> output; // output queue

  sregex_iterator current(infix.begin(), infix.end(), token_regex);
  sregex_iterator end, next = current;
  string prev = "";
  while (current != end) {
    next++;
    //handle toke
    string token = current->str();
    if (regex_match(token.begin(), token.end(), regex(oper))) {
      //token is operator
      push_operator(op_stack, output, token, prev, next);
    } else if (regex_match(token.begin(), token.end(), regex(var_name))) {
      //token is var
      handle_var(output, token, prev, this->mp);
    } else if (regex_match(token.begin(), token.end(), regex(number))) {
      //token is number
      handle_number(output, token);
    } else if (token == ")") {
      handle_closing_bracket(op_stack, output);
    } else if (token == "(") {
      handle_opening_bracket(op_stack, prev);
    }
    // in the else the token is space so we ignore it

    //prepare to the next time
    if (token != " ")
      //the space should not be kept as prev
      prev = token;
    current = next;
  }
  while (!op_stack.empty()) {
    //handling the operators left in the stack
    pop_operator(op_stack, output);
  }
  if (output.size() != 1)
    //there should be one expression on th queue
    throw "wrong syntax";
  return output.top();
}

void push_operator(stack<Operator> &op_stack, stack<Expression*> &output, string op, string prev, sregex_iterator &next) {
   /**
    * takes an operator and if the syntax is legal puts it in the operator stack and pop other operators if needed
    */
    if (regex_match(prev.begin(), prev.end(), regex(oper))) {
      //there cant be 2 operators in a row
      throw "wrong syntax";
    }
    sregex_iterator end;
    //identifying the operator:
    Operator to_push;
    if (prev == "(" || prev.empty()) {
      //unary or negative number
      string next_str = next->str();
      if (next != end && regex_match(next_str.begin(), next_str.end(), regex(number)) && op == "-") {
        //negative number. combine with the next token to one negative number
        handle_number(output, "-" + next->str());
        next++;
        return;
      }
      if (op == "+")
        to_push = Uplus;
      else if (op == "-")
        to_push = Uminus;
      else
        //illegal operator
        throw "wrong syntax";
    } else {
      //binary operator
      if (op == "+")
        to_push = Operator::plus;
      else if (op == "-")
        to_push = Operator::minus;
      else if (op == "*")
        to_push = mul;
      else if (op == "/")
        to_push = Operator::div1;
      else if (op == "==")
          to_push = Operator::equals;
      else if (op == "!=")
          to_push = not_equal;
      else if (op == ">")
          to_push = Operator ::greater;
      else if (op == ">=")
          to_push = Operator::greater_equal;
      else if (op == "<")
          to_push = lower;
      else if (op == "<=")
          to_push = lower_equal;
      else
        //will never get here because of the regex
        throw "wrong syntax";
    }
    while (!op_stack.empty() && priority[op_stack.top()] >= priority[to_push]) {
      pop_operator(op_stack, output);
    }
    op_stack.push(to_push);
}

void handle_var(stack<Expression*> &output, string var, string prev, map<string, double> mp) {
  /**
   * handle a var token and push it to the output
   */
  if (regex_match(prev.begin(), prev.end(), regex(number))) {
    //var cant come after number
    throw "wrong syntax";
  }
  if (mp.find(var) == mp.end())
    //undefined var
    throw "undefined var name";
  output.push(new Variable(var, mp[var]));
}

void handle_number(stack<Expression*> &output, string token) {
  /**
   * handle a number and push it to the output
   */
   const char end[10] = ""; //there will be no end to the strtod because of the regex
   Value * val = new Value(strtod(token.c_str(), (char **) end));
   output.push(val);
}

void handle_closing_bracket(stack<Operator> &op_stack, stack<Expression*> &output) {
  /**
   * pop all operators inside the bracket range
   */
   while (op_stack.top() != bracket) {
     if (op_stack.empty()) {
       //illegal brackets
       throw "wrong syntax";
     }
     pop_operator(op_stack, output);
   }
   op_stack.pop();
}

void handle_opening_bracket(stack<Operator> &op_stack, string prev) {
  /**
   * handle opening bracket by pushing it to the stack
   */
   if (regex_match(prev.begin(), prev.end(), regex(var_name + "|" + number))) {
     //there cant be ( after var/num
     throw "wrong syntax";
   }
   op_stack.push(bracket);
}

void pop_operator(stack<Operator> &op_stack, stack<Expression*> &output) {
  /**
   * pop one operator from the op_stack and push to the output
   */
   if (op_stack.empty())
     throw "wrong syntax";
   Operator op = op_stack.top();
   op_stack.pop();
   Expression* exp1;
   Expression* exp2;
   if (output.empty() || (output.size() == 1 && op !=Uplus && op != Uminus))
     throw "wrong syntax";
   switch (op) {
       case equals:
           exp2 = output.top();
           output.pop();
           exp1 = output.top();
           output.pop();
           output.push(new Equal(exp1, exp2));
           return;
       case not_equal:
           exp2 = output.top();
           output.pop();
           exp1 = output.top();
           output.pop();
           output.push(new NotEqual(exp1, exp2));
           return;
     case Operator ::greater:
           exp2 = output.top();
           output.pop();
           exp1 = output.top();
           output.pop();
           output.push(new Greater(exp1, exp2));
           return;
     case Operator ::greater_equal:
           exp2 = output.top();
           output.pop();
           exp1 = output.top();
           output.pop();
           output.push(new GreaterEqual(exp1, exp2));
           return;
       case lower:
           exp2 = output.top();
           output.pop();
           exp1 = output.top();
           output.pop();
           output.push(new Lower(exp1, exp2));
           return;
       case lower_equal:
           exp2 = output.top();
           output.pop();
           exp1 = output.top();
           output.pop();
           output.push(new LowerEqual(exp1, exp2));
           return;
       case Uminus:
            exp1 = output.top();
            output.pop();
            output.push(new UMinus(exp1));
            return;
       case Uplus:
            exp1 = output.top();
            output.pop();
            output.push(new UPlus(exp1));
            return;
       case Operator ::plus:
          exp2 = output.top();
          output.pop();
          exp1 = output.top();
          output.pop();
          output.push(new Plus(exp1, exp2));
          return;
       case Operator ::minus:
          exp2 = output.top();
          output.pop();
          exp1 = output.top();
          output.pop();
          output.push(new Minus(exp1, exp2));
          return;
       case Operator ::mul:
          exp2 = output.top();
          output.pop();
          exp1 = output.top();
          output.pop();
          output.push(new Mul(exp1, exp2));
          return;
       case Operator ::div1:
          exp2 = output.top();
          output.pop();
          exp1 = output.top();
          output.pop();
          output.push(new Div(exp1, exp2));
          return;
       default:
          //invalid operator
          throw "wrong syntax";
  }
}