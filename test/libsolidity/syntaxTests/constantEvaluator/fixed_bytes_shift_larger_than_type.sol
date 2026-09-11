contract C {
    bytes32 constant x = "abcdefgh";
    uint[(x << 300) ^ 1] y;
}
// ----
// TypeError 6020: (59-73): Operator ^ not compatible with types bytes32 and int_const 1
