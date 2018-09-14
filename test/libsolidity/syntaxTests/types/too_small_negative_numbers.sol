contract C {
  fixed256x76 a = -1e-100;
}
// ----
// TypeError: (31-38): Type rational_const -1 / 1000...(93 digits omitted)...0000 is not implicitly convertible to expected type fixed256x76. Try converting to type fixed8x80 or use an explicit conversion.
