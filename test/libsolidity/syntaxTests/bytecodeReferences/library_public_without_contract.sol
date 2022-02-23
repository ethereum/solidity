library L1 {
    function foo() public { L2.foo(); }

}
library L2 {
    function foo() internal { type(L1).creationCode; }
}
// ----
// TypeError 7813: (99-120): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
