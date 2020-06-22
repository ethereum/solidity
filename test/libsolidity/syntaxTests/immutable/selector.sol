contract C {
    uint immutable x;
    constructor() public {
        x = 3;
        this.readX.selector;
    }

    function readX() external view returns(uint) { return x; }
}
// ----
// Warning 6133: (85-104): Statement has no effect.
// Warning 5805: (85-89): "this" used in constructor. Note that external functions of a contract cannot be called while it is being constructed.
