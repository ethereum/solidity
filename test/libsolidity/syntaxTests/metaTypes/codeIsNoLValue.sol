contract Test {
    function f() public pure {
        type(C).creationCode = new bytes(6);
        type(C).runtimeCode = new bytes(6);
    }
}
contract C {}
// ----
// TypeError 4247: (55-75='type(C).creationCode'): Expression has to be an lvalue.
// TypeError 4247: (100-119='type(C).runtimeCode'): Expression has to be an lvalue.
