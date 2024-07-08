contract C {
    uint public immutable transient x;
}
// ====
// EVMVersion: >=cancun
// ----
// DeclarationError 2197: (17-50): Transient cannot be used as data location for constant or immutable variables.
