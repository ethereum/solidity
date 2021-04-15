library L1 {
    function foo() internal { new A(); }
}
library L2 {
    function foo() internal { L1.foo(); }
}
contract A {
    function f() public pure { L2.foo(); }
}
// ----
// TypeError 7813: (43-48): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
