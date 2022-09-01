contract C {
    int256 public a = -0x4200 >> 8;
}
// ====
// compileToEwasm: also
// ----
// a() -> -66
