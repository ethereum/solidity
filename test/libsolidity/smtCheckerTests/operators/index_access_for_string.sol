pragma experimental SMTChecker;

contract C {
    function f(uint i) public pure {
        string memory x = "\x12\x34";
        bytes memory y = bytes(x);
        assert(y[0] == 0x12);
        assert(y[1] == 0x34);
        require(i > 2);
        assert(y[i] == 0x00);
    }
}
// ----
// Warning 6328: (248-268): CHC: Assertion violation happens here.\nCounterexample:\n\ni = 3\n\n\nTransaction trace:\nconstructor()\nf(3)
