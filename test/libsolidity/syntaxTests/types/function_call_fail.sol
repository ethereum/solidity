contract C {
    function f(uint y) public pure {
        (4(y)) = 2;
    }
}
// ----
// TypeError: (59-63): Type is not callable
// TypeError: (59-63): Expression has to be an lvalue.
// TypeError: (67-68): Type int_const 2 is not implicitly convertible to expected type tuple().
