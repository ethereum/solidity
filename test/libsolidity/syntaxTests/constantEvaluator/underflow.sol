contract C {
    uint8 constant a = 0;
    function f() public pure {
        uint[a - 1] memory x;
    }
}
// ----
// TypeError 2643: (83-88): Arithmetic error when computing constant value.
