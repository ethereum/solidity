contract C {
    function f() public pure {
        bytes32 x;
        x[0] = 0x42;
    }
}
// ----
// TypeError: (71-75): Single bytes in fixed bytes arrays cannot be modified.
