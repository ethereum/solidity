contract C {
    bytes32 constant x = "abcdefgh";
    uint[~x] array;
}
// ----
// TypeError 5462: (59-61): Invalid array length, expected integer literal or constant expression.
