contract C {
    uint public immutable transient x;
}
// ----
// DeclarationError 2197: (17-50): Transient cannot be used as data location for constant or immutable variables.
