contract A is B { }
contract B { function f() public { new C(); } }
contract C { function f() public { new A(); } }
// ----
// TypeError: (14-15): Definition of base has to precede definition of derived contract
