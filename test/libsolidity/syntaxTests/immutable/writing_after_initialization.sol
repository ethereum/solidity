contract C {
    uint immutable x = 0;

    function f() internal {
        x = 1;
    }
}
// ----
// TypeError: (76-77): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError: (76-77): Immutable state variable already initialized.
