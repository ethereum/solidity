contract C { function foo() internal { new D(); } }
contract D { function foo() internal { new C(); } }
// ----
