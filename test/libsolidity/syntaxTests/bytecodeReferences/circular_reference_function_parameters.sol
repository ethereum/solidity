contract C { function foo(D _d) public { _d.foo(this); } }
contract D { function foo(C _c) public { _c.foo(this); } }
// ----
