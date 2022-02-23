contract C {
    uint immutable x = 0;
    uint y = f();

    function f() internal returns(uint) { return x; }
}
// ----
// TypeError 7733: (107-108): Immutable variables cannot be read before they are initialized.
