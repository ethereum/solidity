contract C {
    uint public a = 0x42 << 0x100000000;
}
// ----
// TypeError: (33-52): Operator << not compatible with types int_const 66 and int_const 4294967296
