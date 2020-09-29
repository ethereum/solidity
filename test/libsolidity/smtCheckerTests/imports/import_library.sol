==== Source: c ====
pragma experimental SMTChecker;
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
// ----
// Warning 6328: (c:113-126): CHC: Assertion violation happens here.
// Warning 8364: (c:104-105): Assertion checker does not yet implement type type(library L)
