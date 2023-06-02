contract C {
    uint immutable x = 0;
    uint y = f();

    function f() internal pure returns(uint) { return x; }
}
// ----
// TypeError 7733: (112-113): Immutable variables cannot be read before they are initialized.
