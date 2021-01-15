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
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (117-130): BMC: Assertion violation happens here.
// Warning 4661: (117-130): BMC: Assertion violation happens here.
