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
		require(x < 10); // added to restrict the search space and avoid non-determinsm in Spacer
		x += f();
		assert(y > x);
	}
}
// ----
// Warning 6328: (der:205-218): CHC: Assertion violation happens here.\nCounterexample:\nx = 3, a = 0\ny = 0\n\nTransaction trace:\nDer.constructor()\nState: x = 0, a = 0\nDer.g(0)\n    Base.f() -- internal call
