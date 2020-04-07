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
// Warning: (85-104): Statement has no effect.
// Warning: (114-124): Statement has no effect.
