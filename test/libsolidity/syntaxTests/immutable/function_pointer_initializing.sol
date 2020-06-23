abstract contract B {
    uint immutable x;

    constructor(function() internal returns(uint) fp) {
        x = fp();
    }
}

contract C is B(C.f) {
    function f() internal returns(uint) { return x = 2; }
}
// ----
// TypeError 1581: (200-201): Immutable variables can only be initialized inline or assigned directly in the constructor.
// TypeError 1574: (200-201): Immutable state variable already initialized.
