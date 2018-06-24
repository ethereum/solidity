contract c {
    function f(uint a) external { delete a; }
}
// ----
// TypeError: (54-55): Expression has to be an lvalue.
