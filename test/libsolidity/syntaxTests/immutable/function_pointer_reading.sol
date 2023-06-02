abstract contract B {
    uint immutable x;

    constructor(function() internal returns(uint) fp) {
        x = fp();
    }
}

contract C is B(C.f) {
    function f() internal view returns(uint) { return x + 2; }
}
// ----
// TypeError 7733: (205-206): Immutable variables cannot be read before they are initialized.
