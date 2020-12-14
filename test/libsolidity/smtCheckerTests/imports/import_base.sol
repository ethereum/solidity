==== Source: base ====
contract Base {
	uint x;
	address a;
	function f() internal returns (uint) {
		a = address(this);
		++x;
		return 2;
	}
}
==== Source: der ====
pragma experimental SMTChecker;
import "base";
contract Der is Base {
	function g(uint y) public {
		x += f();
		assert(y > x);
	}
}
// ----
// Warning 4984: (der:101-109): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6328: (der:113-126): CHC: Assertion violation happens here.\nCounterexample:\nx = 3, a = 0\ny = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 0, a = 0\ng(0)
// Warning 4984: (base:100-103): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 2661: (base:100-103): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (der:101-109): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (base:100-103): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
