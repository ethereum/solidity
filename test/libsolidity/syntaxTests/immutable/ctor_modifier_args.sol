contract C {
    uint immutable x;
    constructor() readX(x = 3) { }

    modifier readX(uint _x) {
        _; f(_x);
    }

    function f(uint a) internal pure {}
}
// ----
