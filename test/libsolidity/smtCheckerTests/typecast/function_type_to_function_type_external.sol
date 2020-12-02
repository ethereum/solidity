pragma experimental SMTChecker;
contract C {
    function f(function(uint) external returns (uint) g, function(uint) external returns (uint) h) public {
		assert(g(2) == h(2));
    }
}
// ----
// Warning 6328: (155-175): CHC: Assertion violation happens here.\nCounterexample:\n\ng = 0\nh = 0\n\n\nTransaction trace:\nconstructor()\nf(0, 0)
