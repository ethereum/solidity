contract C {
    function f(uint y) public pure {
        (4(y)) = 2;
    }
}
// ----
// TypeError: (59-63): Type is not callable
