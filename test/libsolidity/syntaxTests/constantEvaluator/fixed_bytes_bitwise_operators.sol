contract C {
    bytes32 constant x = "abcdefgh";
    bytes32 constant y = hex"00ffffffffffffff";

    uint[((x & y) | (x ^ y)) & 1] z;
}
// ----
// TypeError 6020: (108-131): Operator & not compatible with types bytes32 and int_const 1
