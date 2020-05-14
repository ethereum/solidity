contract test {
    int8 public x = 2;
    int8 public y = 127;
    int16 public q = 250;
}
// ====
// compileViaYul: also
// ----
// x() -> 2
// y() -> 127
// q() -> 250
