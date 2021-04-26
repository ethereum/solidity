contract C {
    int256 public a = -0x42 << 8;
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// a() -> -16896
