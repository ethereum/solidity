contract C {
    function f() public {
        delete f;
    }
}
// ----
// TypeError: (54-55): Expression has to be an lvalue.
