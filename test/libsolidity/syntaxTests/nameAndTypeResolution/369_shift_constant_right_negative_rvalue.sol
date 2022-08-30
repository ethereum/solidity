contract C {
    uint public a = 0x42 >> -8;
}
// ----
// TypeError 2271: (33-43): Binary operator >> not compatible with types int_const 66 and int_const -8.
