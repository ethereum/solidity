pragma experimental SMTChecker;

contract C {
    function f() public pure {
        bytes memory y = "def";
        y[0] &= "d";
        assert(y[0] == "d");

        y[0] |= "e";
        assert(y[0] == "d"); // fails

        y[0] ^= "f";
        assert(y[0] == (bytes1("d") | bytes1("e")) ^ bytes1("f"));
    }
}
// ----
// Warning 6328: (189-208): CHC: Assertion violation happens here.
