pragma experimental SMTChecker;
contract C {
    function f() public pure {}
    constructor() public {
        C c = this;
        c.f(); // this does not warn now, but should warn in the future
        this.f();
        (this).f();
    }
}
// ----
// Warning: (204-208): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
// Warning: (223-227): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
