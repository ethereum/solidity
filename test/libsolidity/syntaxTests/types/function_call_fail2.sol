contract C {
    function f(uint y) public pure returns (uint) {
        (f(y)) = 2;
    }
}
// ----
// TypeError 4247: (74-78): Expression has to be an lvalue.
