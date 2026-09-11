contract C {
    uint[keccak256("test") & 1] and;
    uint[keccak256("test") | 1] or;
    uint[keccak256("test") ^ 1] xor;
    uint[~keccak256("test")] not;
}
// ----
// TypeError 6020: (22-43): Operator & not compatible with types bytes32 and int_const 1
