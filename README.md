# flight-simulator
this is an interpreter for flightgear. the main should receive a text file name, which will contain commands for the interpreter.
the possible commands are:
* openDataServer(port) - create a server on another thread that listen on port "port", read values from simulator and update vars that read from simulator.
* connectControlClient(ip,port) - craete a client on another thread that connects to the simulator at (ip, port) and send values from vars that write to the simulator.
*var name = value - create a var in the scope with value of "value"
*var name -> sim(path) - create a var in scope that write to simulator to the value path.
*var name <- sim(path) - create a var in scope that read from simulator from the value path.
*x = value - assign value to x.
*while condition { - create a while loop while the condition is true, and create an inner block with its own scope.
*if condition { - if condition is true create an inner block with its own scope and run it.
*} - close a block.
*Sleep(time) - sleep for time miliseconds
*Print(mess) - print mess. can be string or value.


every value and condition can be expression that containes doubles, vars from the scope, and the operators:
+, -, *, /, ), (, ==, !=, <, >, <=, >=.
each scope can access the outer scope, if a var is defined both in inner and outer scope, the inner scope var will hide the outer scope var.
when a scope is ending, all vars in it are deleted.
my implementation can support a condition command (if or while) inside another.
