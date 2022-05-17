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
// Warning 6328: (229-276): CHC: Assertion violation happens here.
// Warning 6328: (394-437): CHC: Assertion violation might happen here.
// Warning 4661: (394-437): BMC: Assertion violation happens here.
