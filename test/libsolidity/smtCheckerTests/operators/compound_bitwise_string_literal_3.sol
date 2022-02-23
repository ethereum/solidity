contract C {
    function f() public pure {
        bytes32 y = "abcdefghabcdefghabcdefghabcdefgh";
        bytes32 z = y;
        y &= "bcdefghabcdefghabcdefghabcdefgha";
        z &= "bcdefghabcdefghabcdefghabcdefgha";
        assert(y == "abcdefghabcdefghabcdefghabcdefgh"); // fails

        y |= "cdefghabcdefghabcdefghabcdefghab";
        z |= "cdefghabcdefghabcdefghabcdefghab";
        assert(y == "abcdefghabcdefghabcdefghabcd"); // fails

        y ^= "abcdefghabcdefghabcdefghabcdefgh";
        assert(y == z ^ "abcdefghabcdefghabcdefghabcdefgh");
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (229-276): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 0x6062606464666060606260646466606060626064646660606062606464666060\nz = 0x6062606464666060606260646466606060626064646660606062606464666060\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (394-437): CHC: Assertion violation might happen here.
// Warning 4661: (394-437): BMC: Assertion violation happens here.
