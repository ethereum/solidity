pragma experimental SMTChecker;

contract C {
    function f(uint i) public pure {
        bytes memory x = hex"00112233";
        assert(x[0] == 0x00);
        assert(x[1] == 0x11);
        require(i > 3);
        assert(x[i] == 0x00);
    }
}
// ----
// Warning 6328: (215-235): CHC: Assertion violation happens here.\nCounterexample:\n\ni = 4\n\n\nTransaction trace:\nconstructor()\nf(4)
