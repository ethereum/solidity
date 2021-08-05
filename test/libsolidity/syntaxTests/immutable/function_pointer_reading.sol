abstract contract B {
    uint immutable x;

    constructor(function() internal returns(uint) fp) {
        x = fp();
    }
}

contract C is B(C.f) {
    function f() internal returns(uint) { return x + 2; }
}
// ----
// TypeError 7733: (200-201): Immutable variables cannot be read before they are initialized.
