contract test {
    int8 public i = -129;
}
// ----
// TypeError 7407: (36-40): Type int_const -129 is not implicitly convertible to expected type int8. Literal is too large to fit in int8.
