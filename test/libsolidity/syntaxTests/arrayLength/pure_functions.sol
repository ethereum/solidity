contract C {
    uint constant LEN = keccak256(ripemd160(33));
    uint[LEN] ids;
}
// ----
// TypeError: (72-75): Invalid array length, expected integer literal or constant expression.
