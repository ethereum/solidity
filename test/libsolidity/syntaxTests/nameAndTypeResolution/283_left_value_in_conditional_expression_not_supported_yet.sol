contract C {
    function f() public {
        uint x;
        uint y;
        (true ? x : y) = 1;
    }
}
// ----
// TypeError 2212: (80-92='true ? x : y'): Conditional expression as left value is not supported yet.
// TypeError 4247: (80-92='true ? x : y'): Expression has to be an lvalue.
