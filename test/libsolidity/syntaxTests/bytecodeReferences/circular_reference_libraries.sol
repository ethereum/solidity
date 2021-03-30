library L1 { function foo() internal { L2.foo(); } }
library L2 { function foo() internal { L1.foo(); } }
// ----
