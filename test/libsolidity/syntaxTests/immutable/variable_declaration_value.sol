contract C {
    int immutable x = x = 5;
}
// ----
// TypeError 1581: (35-36): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError 1574: (35-36): Immutable state variable already initialized.
