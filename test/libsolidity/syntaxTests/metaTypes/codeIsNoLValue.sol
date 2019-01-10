contract Test {
    function f() public pure {
        type(C).creationCode = new bytes(6);
        type(C).runtimeCode = new bytes(6);
    }
}
contract C {}
// ----
// TypeError: (55-75): Expression has to be an lvalue.
// TypeError: (100-119): Expression has to be an lvalue.
