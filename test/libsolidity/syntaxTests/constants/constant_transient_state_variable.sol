contract C {
    int constant public transient x = 0;
}
// ----
// DeclarationError 2197: (17-52): Transient cannot be used as data location for constant or immutable variables.
