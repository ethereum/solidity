contract B { function f() public {} }
contract C is B { function f(uint i) public {} }
// ----
// Warning: (67-73): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (13-35): Function state mutability can be restricted to pure
// Warning: (56-84): Function state mutability can be restricted to pure
