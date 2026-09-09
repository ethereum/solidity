contract C {
    int[uint128(42)] a;
    int[uint8(8)] b;
    int[uint64(uint(64))] c;
}
// ----
// TypeError 5462: (21-32): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (45-53): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (66-82): Invalid array length, expected integer literal or constant expression.
