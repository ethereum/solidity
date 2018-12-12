contract A { function f(uint a) public {} }
contract B { function f() public {} }
contract C is A, B { }
// ----
// Warning: (13-41): Function state mutability can be restricted to pure
// Warning: (57-79): Function state mutability can be restricted to pure
