abstract contract A { function f() public pure virtual; }
contract B is A { function f() public pure virtual override {} }
contract C is A, B { }
contract D is A, B { function f() public pure override(A, B) {} }
// ----
