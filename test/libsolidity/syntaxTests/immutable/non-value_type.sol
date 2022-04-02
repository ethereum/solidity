contract C {
    uint[] immutable x;
}
// ----
// TypeError 6377: (17-35='uint[] immutable x'): Immutable variables cannot have a non-value type.
