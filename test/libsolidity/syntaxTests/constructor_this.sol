contract C {
    function f() public pure {}
    constructor() {
        C c = this;
        c.f(); // this does not warn now, but should warn in the future
        this.f();
        (this).f();
    }
}
// ----
// Warning 5805: (165-169): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
// Warning 5805: (184-188): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
