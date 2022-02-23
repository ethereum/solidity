contract C {
    function f() public {
        delete this.f;
    }
}
// ----
// TypeError 4247: (54-60): Expression has to be an lvalue.
