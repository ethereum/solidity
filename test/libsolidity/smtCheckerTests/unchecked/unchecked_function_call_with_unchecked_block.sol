pragma experimental SMTChecker;
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
// ----
// Warning 6328: (117-130): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\nTransaction trace:\nC.constructor()\nC.g(0)\n    C.f(0) -- internal call
