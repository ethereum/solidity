contract C {
    uint constant b3 = 1 % 0;
}
// ----
// TypeError: (36-41): Operator % not compatible with types int_const 1 and int_const 0
