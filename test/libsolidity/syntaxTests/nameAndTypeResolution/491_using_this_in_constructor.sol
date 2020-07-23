contract C {
    constructor() {
        this.f();
    }
    function f() pure public {
    }
}
// ----
// Warning 5805: (41-45): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
