//
// Created by tomer on 10.11.2019.
//

#ifndef EX1_EX1_H
#define EX1_EX1_H

#include <string>
#include <regex>
#include "Expression.h"
#include "Interpreter.h"

class Value : public Expression {
  double val;
 public:
  explicit Value(double value) : val(value) {};
  virtual double calculate() { return this->val; }
};

class UnaryOperator : public Expression {
 protected:
  Expression *exp;
 public:
  explicit UnaryOperator(Expression *ex) : exp(ex) {};
  virtual ~UnaryOperator() { delete exp; }
};

class UPlus : public UnaryOperator {
 public:
  explicit UPlus(Expression *ex) : UnaryOperator(ex) {};
  virtual double calculate();
};

class UMinus : public UnaryOperator {
 public:
  explicit UMinus(Expression *ex) : UnaryOperator(ex) {};
  virtual double calculate();
};

class BinaryOperator : public Expression {
 protected:
  Expression *left;
  Expression *right;
 public:
  BinaryOperator(Expression *l, Expression *r) : left(l), right(r) {};
  virtual ~BinaryOperator() {
    delete left;
    delete right;
  }
};

class Plus : public BinaryOperator {
 public:
  Plus(Expression *l, Expression *r) : BinaryOperator(l, r) {};
  virtual double calculate();
};

class Minus : public BinaryOperator {
 public:
  Minus(Expression *l, Expression *r) : BinaryOperator(l, r) {};
  virtual double calculate();
};

class Mul : public BinaryOperator {
 public:
  Mul(Expression *l, Expression *r) : BinaryOperator(l, r) {};
  virtual double calculate();
};

class Div : public BinaryOperator {
 public:
  Div(Expression *l, Expression *r) : BinaryOperator(l, r) {};
  virtual double calculate();
};

class Variable : public Expression {
  string name;
  double value;
 public:
  Variable(string n, double v) : name(std::move(n)), value(v) {};
  Variable &operator++();
  Variable &operator++(int);
  Variable &operator--();
  Variable &operator--(int);
  Variable &operator+=(double x);
  Variable &operator-=(double x);
  double calculate() { return this->value;}

};

class Equal: public BinaryOperator {
public:
    Equal(Expression *l, Expression *r) : BinaryOperator(l, r) {};
    virtual double calculate();
};

class NotEqual: public BinaryOperator {
public:
    NotEqual(Expression *l, Expression *r) : BinaryOperator(l, r) {};
    virtual double calculate();
};

class Greater: public BinaryOperator {
public:
    Greater(Expression *l, Expression *r) : BinaryOperator(l, r) {};
    virtual double calculate();
};

class GreaterEqual: public BinaryOperator {
public:
    GreaterEqual(Expression *l, Expression *r) : BinaryOperator(l, r) {};
    virtual double calculate();
};

class Lower: public BinaryOperator {
public:
    Lower(Expression *l, Expression *r) : BinaryOperator(l, r) {};
    virtual double calculate();
};

class LowerEqual: public BinaryOperator {
public:
    LowerEqual(Expression *l, Expression *r) : BinaryOperator(l, r) {};
    virtual double calculate();
};
#endif //EX1_EX1_H
