contract C {
    int constant public transient x = 0;
}
// ====
// EVMVersion: >=cancun
// ----
// DeclarationError 2197: (17-52): Transient cannot be used as data location for constant or immutable variables.
// DeclarationError 9825: (17-52): Initialization of transient storage state variables is not supported.
