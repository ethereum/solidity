contract A { function f() public { uint8 x = C(0).g(); } }
contract B { function f() public {} function g() public returns (uint8) {} }
contract C is A, B { }
// ----
// Warning: (35-42): Unused local variable.
// Warning: (95-133): Function state mutability can be restricted to pure
