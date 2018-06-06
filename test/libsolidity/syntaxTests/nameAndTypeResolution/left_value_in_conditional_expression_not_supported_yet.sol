contract C {
    function f() public {
        uint x;
        uint y;
        (true ? x : y) = 1;
    }
}
// ----
// TypeError: (80-92): Conditional expression as left value is not supported yet.
// TypeError: (80-92): Expression has to be an lvalue.
