contract C {
    function f() public pure {
        bytes32 x;
        x[0] = 0x42;
    }
}
// ----
// TypeError 4360: (71-75): Single bytes in fixed bytes arrays cannot be modified.
