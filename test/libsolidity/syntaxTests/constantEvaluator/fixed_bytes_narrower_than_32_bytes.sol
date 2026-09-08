contract C {
    bytes4 constant x = "abcdefgh";
    uint[x & 1] y;
}
// ----
// TypeError 5462: (58-63): Invalid array length, expected integer literal or constant expression.
