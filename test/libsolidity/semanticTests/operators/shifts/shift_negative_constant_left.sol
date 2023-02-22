contract C {
    int256 public a = -0x42 << 8;
}
// ----
// a() -> -16896
