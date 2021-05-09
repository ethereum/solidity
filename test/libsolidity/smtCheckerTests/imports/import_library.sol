==== Source: c ====
import "lib";
contract C {
	function g(uint x) public pure {
		uint y = L.f();
		assert(x > y);
	}
}
==== Source: lib ====
library L {
	uint constant one = 1;
	function f() internal pure returns (uint) {
		return one;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (c:81-94): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 1\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    L.f() -- internal call
