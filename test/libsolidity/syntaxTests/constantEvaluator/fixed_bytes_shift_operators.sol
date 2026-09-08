contract C {
    bytes32 constant fixedBytes = "abcdefgh";
    uint8 constant shiftAmount = 8;
    uint[fixedBytes << shiftAmount] x;
    uint[fixedBytes >> shiftAmount] y;
}
// ----
// TypeError 5462: (104-129): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (143-168): Invalid array length, expected integer literal or constant expression.
