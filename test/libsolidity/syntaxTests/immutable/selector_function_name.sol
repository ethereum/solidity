contract C {
    uint immutable x;
    constructor() public {
        x = 3;
        C.selector.selector;
        C.selector;
    }

    function selector() external view returns(uint) { return x; }
}
// ----
// Warning 6133: (85-104): Statement has no effect.
// Warning 6133: (114-124): Statement has no effect.
