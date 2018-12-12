contract B { function f() public {} }
contract C is B { function f(uint i) public {} }
// ----
// Warning: (13-35): Function state mutability can be restricted to pure
// Warning: (56-84): Function state mutability can be restricted to pure
