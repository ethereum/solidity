contract A { function f() public virtual { uint8 x = C(address(0)).g(); } }
contract B { function f() public virtual {} function g() public returns (uint8) {} }
contract C is A, B { function f() public override (A, B) { A.f(); } }
// ----
// Warning 2072: (43-50): Unused local variable.
