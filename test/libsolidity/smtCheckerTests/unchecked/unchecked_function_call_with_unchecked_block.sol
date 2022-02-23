contract C {
	function f(uint x) internal pure {
		unchecked {
			uint y = x - 1;
			assert(y < x); // should fail, underflow can happen, we are inside unchecked block
		}
	}
	function g(uint x) public pure {
		unchecked { f(x); }
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (85-98): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 115792089237316195423570985008687907853269984665640564039457584007913129639935\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    C.f(0) -- internal call
