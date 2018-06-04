contract C {
    constructor() public {
        this.f();
    }
    function f() pure public {
    }
}
// ----
// Warning: (48-52): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
