contract c {
    function f(uint a) external { a++; }
}
// ----
// TypeError: (47-48): Expression has to be an lvalue.
