contract C {
    uint public a = 0x42 << 0x100000000;
}
// ----
// TypeError 2271: (33-52): Binary operator << not compatible with types int_const 66 and int_const 4294967296.
