pragma experimental SMTChecker;

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
// ----
// Warning 6328: (262-309): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (427-470): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
