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
// TypeError 4224: (133-152): Circular reference for contract code access.
