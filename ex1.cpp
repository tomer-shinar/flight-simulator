//
// Created by tomer on 10.11.2019.
//
#include "ex1.h"


//implement the calculate method for the unary and binary expressions:
double UPlus::calculate() {
  return this->exp->calculate();
}

double UMinus::calculate() {
  return -this->exp->calculate();
}

double Plus::calculate() {
  return this->left->calculate() + this->right->calculate();
}

double Minus::calculate() {
  return this->left->calculate() - this->right->calculate();
}

double Mul::calculate() {
  return this->left->calculate() * this->right->calculate();
}

double Div::calculate() {
  if (this->right->calculate() == 0)
    throw "cant divide by 0";
  return this->left->calculate() / this->right->calculate();
}

//implement the operator overloading for Variable:
Variable & Variable::operator++() {
  this->value++;
  return *this;
}

Variable & Variable::operator++(int) {
  this->value++;
  return *this;
}

Variable & Variable::operator--() {
  this->value--;
  return *this;
}

Variable & Variable::operator--(int) {
  this->value--;
  return *this;
}

Variable & Variable::operator+=(double x) {
  this->value += x;
  return *this;
}

Variable & Variable::operator-=(double x) {
  this->value -= x;
  return *this;
}

double Equal::calculate() {
    return this->left->calculate() == this->right->calculate();
}

double NotEqual::calculate() {
    return this->left->calculate() != this->right->calculate();
}

double Greater::calculate() {
    return this->left->calculate() > this->right->calculate();
}

double GreaterEqual::calculate() {
    return this->left->calculate() >= this->right->calculate();
}

double Lower::calculate() {
    return this->left->calculate() < this->right->calculate();
}

double LowerEqual::calculate() {
    return this->left->calculate() <= this->right->calculate();
}