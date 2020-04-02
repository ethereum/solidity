contract C {
    uint immutable x = 0;

    function f() readX internal { }

    modifier readX() {
        _; x = 1;
    }
}
// ----
// TypeError: (111-112): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError: (111-112): Immutable state variable already initialized.
