struct S {
    function() external e;
}

contract C {
    S s;
    function() external f;

    constructor() {
        delete s.e;
        assert(s.e == f); // should fail for now because function pointer comparisons are not supported
    }
}
// ----
// Warning 7229: (146-154): Assertion checker does not yet implement the type function () external for comparisons
// Warning 6328: (139-155): CHC: Assertion violation happens here.\nCounterexample:\ns = {e: 0}, f = 0\n\nTransaction trace:\nC.constructor()
