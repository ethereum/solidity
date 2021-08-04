contract C {
  fixed a = -1e-100;
}
// ----
// TypeError 2326: (25-32): Type rational_const -1 / 1000...(93 digits omitted)...0000 is not implicitly convertible to expected type fixed128x18. Try converting to type fixed8x80 or use an explicit conversion.
