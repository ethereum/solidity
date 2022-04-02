contract C {
    uint immutable x;
    constructor() {
        x = 3;
        C.selector.selector;
        C.selector;
    }

    function selector() external view returns(uint) { return x; }
}
// ----
// Warning 6133: (78-97='C.selector.selector'): Statement has no effect.
// Warning 6133: (107-117='C.selector'): Statement has no effect.
