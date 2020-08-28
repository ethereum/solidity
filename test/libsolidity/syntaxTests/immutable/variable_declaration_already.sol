contract C {
    uint immutable z = 2;
    uint immutable x = z = y = 3;
    uint immutable y = 5;
}
// ----
// TypeError 1581: (62-63): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError 1574: (62-63): Immutable state variable already initialized.
// TypeError 1581: (66-67): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError 1574: (66-67): Immutable state variable already initialized.
