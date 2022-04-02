contract A {
    function f() public pure {
        type(B).runtimeCode;
    }
}
contract B {
    function f() public pure {
        type(A).runtimeCode;
    }
}
// ----
// TypeError 7813: (52-71='type(B).runtimeCode'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (133-152='type(A).runtimeCode'): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
