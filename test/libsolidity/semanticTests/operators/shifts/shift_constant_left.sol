contract C {
    uint256 public a = 0x42 << 8;
}
// ====
// compileViaYul: also
// ----
// a() -> 0x4200
