contract C {
    uint public a = 0x42 >> (1 / 2);
}
// ----
// TypeError 2271: (33-48): Built-in binary operator >> cannot be applied to types int_const 66 and rational_const 1 / 2.
