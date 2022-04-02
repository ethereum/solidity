contract C {
    function f(uint y) public pure {
        (4(y)) = 2;
    }
}
// ----
// TypeError 5704: (59-63='4(y)'): Type is not callable
