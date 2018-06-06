contract C {
    uint public a = 0x42 >> -8;
}
// ----
// TypeError: (33-43): Operator >> not compatible with types int_const 66 and int_const -8
