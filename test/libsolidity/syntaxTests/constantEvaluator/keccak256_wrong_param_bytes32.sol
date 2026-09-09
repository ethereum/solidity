contract C {
    bytes32 constant b32 = "1234abcd";
    uint constant BYTES32 = uint(keccak256(b32));
    uint[BYTES32] array;
}
// ----
// TypeError 5462: (111-118): Invalid array length, expected integer literal or constant expression.
