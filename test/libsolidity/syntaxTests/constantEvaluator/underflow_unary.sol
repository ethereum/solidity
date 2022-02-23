contract C {
    int8 constant a = -128;
    function f() public pure {
        uint[-a] memory x;
    }
}
// ----
// TypeError 3667: (85-87): Arithmetic error when computing constant value.
