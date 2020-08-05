contract C {
    uint immutable x;
    constructor() {
        x = 3;
        this.readX.selector;
    }

    function readX() external view returns(uint) { return x; }
}
// ----
// Warning 6133: (78-97): Statement has no effect.
// Warning 5805: (78-82): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
