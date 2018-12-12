contract A { function f(uint a) public {} }
contract B { function f() public {} }
contract C is A, B { }
// ----
