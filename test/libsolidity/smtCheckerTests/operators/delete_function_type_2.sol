contract C {
    mapping (uint => function() external) m;
    function() external f;

    constructor() {
        m[2] = f;
        delete m[2];
        assert(m[2] == f); // should fail for now because function pointer comparisons are not supported
    }
}
// ----
// Warning 7229: (160-169): Assertion checker does not yet implement the type function () external for comparisons
// Warning 6328: (153-170): CHC: Assertion violation happens here.\nCounterexample:\nf = 0\n\nTransaction trace:\nC.constructor()
