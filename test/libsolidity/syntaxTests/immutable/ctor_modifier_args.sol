contract C {
    uint immutable x;
    constructor() readX(x = 3) { }

    modifier readX(uint _x) {
        _; f(_x);
    }

    function f(uint a) internal pure {}
}
// ----
// TypeError 1581: (59-60): Cannot write to immutable here: Immutable variables can only be initialized inline or assigned directly in the constructor.
