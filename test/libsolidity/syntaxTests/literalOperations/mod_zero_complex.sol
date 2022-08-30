contract C {
    uint constant b3 = 1 % (-4+((2)*2));
}
// ----
// TypeError 2271: (36-52): Binary operator % not compatible with types int_const 1 and int_const 0.
