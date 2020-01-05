//
// Created by tomer on 24/12/2019.
//

#ifndef FLIGHT_SIMULATOR__VAR_H_
#define FLIGHT_SIMULATOR__VAR_H_

#include <string>
#include <utility>

using namespace std;

class Var {
  /**
   * class to represent var in the interpreter
   */
 private:
  string name;
  double value;
 public:
  Var(string name, double value) :name(std::move(name)), value(value) {};
  string get_name() {return this->name;}
  double get_value() {return this->value;}
  virtual void set_value(double new_val) {this->value = new_val;}
  virtual ~Var() = default;
};

class WritingVar: public Var {
  /**
   * vars that their values are writing to the simulator
   */
 private:
  string write_to;
  bool new_data = false;
 public:
  WritingVar(string name, string target): Var(name, 0), write_to(target) {};
  virtual void set_value(double new_val) {
    new_data = true;
    Var::set_value(new_val);
  }
  string getTarget() {return this->write_to;}
  bool newData() {return this->new_data;}
  void dataRead() {
    //the new data have been read
    this->new_data = false;
  }
};

class ReadingVar: public Var {
  /**
   * vars that their values are read from the simulator
   */
 private:
  string read_from;
 public:
  ReadingVar(string name, string src): Var(name, 0), read_from(src) {};
  string getSource() {return read_from;}
};

#endif //FLIGHT_SIMULATOR__VAR_H_
