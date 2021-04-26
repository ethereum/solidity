contract C {
    uint256 public a = 0x4200 >> 8;
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// a() -> 0x42
