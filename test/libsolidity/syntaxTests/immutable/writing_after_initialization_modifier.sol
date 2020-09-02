contract C {
    uint immutable x = 0;

    function f() readX internal { }

    modifier readX() {
        _; x = 1;
    }
}
// ----
// TypeError 1581: (111-112): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
