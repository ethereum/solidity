contract B {
    uint immutable x;

    constructor(function() internal returns(uint) fp) internal {
        x = fp();
    }
}

contract C is B(C.f) {
    function f() internal returns(uint) { return x = 2; }
}
// ----
// TypeError: (200-201): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError: (200-201): Immutable state variable already initialized.
