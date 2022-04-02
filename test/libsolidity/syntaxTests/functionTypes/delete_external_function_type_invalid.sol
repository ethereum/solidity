contract C {
    function f() public {
        delete this.f;
    }
}
// ----
// TypeError 4247: (54-60='this.f'): Expression has to be an lvalue.
