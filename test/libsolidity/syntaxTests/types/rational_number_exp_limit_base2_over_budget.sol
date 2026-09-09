contract C { uint256 constant X = (2 ** 4096) / (2 ** 4096); }
// ----
// TypeError 2271: (35-44): Built-in binary operator ** cannot be applied to types int_const 2 and int_const 4096.
// TypeError 2271: (49-58): Built-in binary operator ** cannot be applied to types int_const 2 and int_const 4096.
