contract C {
    function f(function(uint) external returns (uint) g, function(uint) external returns (uint) h) public {
		assert(g(2) == h(2));
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (123-143): CHC: Assertion violation happens here.\nCounterexample:\n\ng = 0\nh = 0\n\nTransaction trace:\nC.constructor()\nC.f(0, 0)\n    g(2) -- untrusted external call\n    h(2) -- untrusted external call
