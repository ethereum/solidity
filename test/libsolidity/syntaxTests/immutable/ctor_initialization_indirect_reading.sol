contract C {
    uint immutable x;
    constructor() {
        x = f();
    }

    function f() public view returns (uint) { return 3 + x; }
}
// ----
// TypeError 7733: (136-137): Immutable variables cannot be read before they are initialized.
