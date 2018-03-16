contract C {
    function f() public {
        delete this.f;
    }
}
// ----
// TypeError: (54-60): Expression has to be an lvalue.
