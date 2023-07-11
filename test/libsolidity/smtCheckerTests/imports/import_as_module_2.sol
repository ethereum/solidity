==== Source: A ====
import "s1.sol" as M;
function f(uint _x) pure {
	assert(_x > 0);
}
contract D {
	function g(uint _y) public pure {
		M.f(200); // should hold
		M.f(_y); // should fail
		f(10); // should hold
		f(_y); // should fail
	}
}
==== Source: s1.sol ====
function f(uint _x) pure {
	assert(_x > 100);
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (A:50-64): CHC: Assertion violation happens here.\nCounterexample:\n\n_y = 0\n\nTransaction trace:\nD.constructor()\nD.g(0)\n    s1.sol:f(200) -- internal call\n    s1.sol:f(0) -- internal call\n    A:f(10) -- internal call\n    A:f(0) -- internal call
// Warning 6328: (s1.sol:28-44): CHC: Assertion violation happens here.\nCounterexample:\n\n_y = 0\n\nTransaction trace:\nD.constructor()\nD.g(0)\n    s1.sol:f(200) -- internal call\n    s1.sol:f(0) -- internal call
