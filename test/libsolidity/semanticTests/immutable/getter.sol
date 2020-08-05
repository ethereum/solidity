contract C {
    uint immutable public x = 1;
}
// ====
// compileViaYul: also
// ----
// x() -> 1
