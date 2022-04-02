contract C {
    function f() public {
        delete f;
    }
}
// ----
// TypeError 4247: (54-55='f'): Expression has to be an lvalue.
