contract c {
    function f(uint a) external { a = 1; }
}
// ----
// TypeError: (47-48): Expression has to be an lvalue.
