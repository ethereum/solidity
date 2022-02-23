library L1 {
    function foo() internal { new A(); }
}
library L2 {
    function foo() public { L1.foo(); }
}
contract A {
    function f() public { L2.foo(); }
}
// ----
