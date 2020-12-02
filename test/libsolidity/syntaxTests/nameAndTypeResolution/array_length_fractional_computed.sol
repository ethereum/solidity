contract test {
    uint constant a = 7;
    uint constant b = 3;
    function f() public {
        uint[a / b] memory x; x[0];
        uint[7 / 3] memory y; y[0];
    }
}
// ----
// TypeError 3208: (141-146): Array with fractional length specified.
