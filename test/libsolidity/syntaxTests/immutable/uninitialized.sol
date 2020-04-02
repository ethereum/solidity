contract C {
    uint immutable x;
}
// ----
// TypeError: (0-36): Construction control flow ends without initializing all immutable state variables.
