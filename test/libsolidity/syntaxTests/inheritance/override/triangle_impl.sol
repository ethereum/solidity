contract A { function f() public pure virtual {} }
contract B is A { function f() public pure virtual override {} }
contract C is A, B { }
contract D is A, B { function f() public pure override(A, B) {} }
// ----
// TypeError 6480: (116-138): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
