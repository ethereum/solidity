contract C {
    uint8 constant a = 255;
    uint16 constant b = a + 2;
    function f() public pure {
        uint[b] memory x;
    }
}
// ----
// TypeError 2643: (65-70): Arithmetic error when computing constant value.
