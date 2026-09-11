contract C {
    uint[keccak256("test") << 8] shiftL;
    uint[keccak256("test") >> 16] shiftR;
}
// ----
// TypeError 5462: (22-44): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (63-86): Invalid array length, expected integer literal or constant expression.
