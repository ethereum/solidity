pragma experimental SMTChecker;

contract C {
    function f(uint a) internal pure returns (uint b) {
        require(a < 1000);
        return a * a;
    }
    function g(uint a) internal pure returns (uint b) {
        require(a < 1000);
        return a + 100;
    }
    function h(uint a) public pure {
        uint c = a < 5 ? g(a) : f(a);
        assert(c >= 25);
        assert(c < 20); // should fail
    }
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (378-392): CHC: Assertion violation happens here.
