contract C {
  fixed8x80 a = -1e-100;
}
// ----
// TypeError 7407: (29-36): Type rational_const -1 / 1000...(93 digits omitted)...0000 is not implicitly convertible to expected type fixed8x80. Conversion incurs precision loss. Use an explicit conversion instead.
