contract C {
    uint[] immutable x;
}
// ----
// TypeError: (17-35): Immutable variables cannot have a non-value type.
