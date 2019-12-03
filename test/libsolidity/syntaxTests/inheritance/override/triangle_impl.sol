contract A { function f() public pure virtual {} }
contract B is A { function f() public pure virtual override {} }
contract C is A, B { }
contract D is A, B { function f() public pure override(A, B) {} }
// ----
// TypeError: (116-138): Derived contract must override function "f". Function with the same name and parameter types defined in two or more base classes.
