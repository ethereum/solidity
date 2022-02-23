contract C {
    int8 constant a = -7;
    function f() public pure {
        uint[-a] memory x;
        x[0] = 2;
    }
}
// ----
