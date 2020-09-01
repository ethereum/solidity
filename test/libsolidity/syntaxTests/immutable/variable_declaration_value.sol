contract C {
    int immutable x = x = 5;
}
// ----
// TypeError 1581: (35-36): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
