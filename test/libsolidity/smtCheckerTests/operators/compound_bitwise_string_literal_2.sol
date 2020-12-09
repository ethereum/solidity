pragma experimental SMTChecker;

contract C {
    function f() public pure {
        bytes3 y = "def";
        y &= "def";
        assert(y == "def");

        y |= "dee";
        assert(y == "def"); // fails

        y ^= "fed";
        assert(y == (bytes3("def") | bytes3("dee")) ^ bytes3("fed"));
    }
}
// ----
// Warning 6328: (180-198): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
