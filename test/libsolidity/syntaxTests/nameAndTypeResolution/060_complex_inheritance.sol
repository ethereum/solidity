contract A { function f() public { uint8 x = C(0).g(); } }
contract B { function f() public {} function g() public returns (uint8) {} }
contract C is A, B { function f() public override (A, B) { A.f(); } }
// ----
// Warning: (35-42): Unused local variable.
