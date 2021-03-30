contract Test {
    function f() public { Test x = new Test(); }
}
// ----
// TypeError 7813: (51-59): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
