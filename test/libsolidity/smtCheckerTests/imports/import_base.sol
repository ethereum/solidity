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
import "base";
contract Der is Base {
	function g(uint y) public {
		require(x < 10); // added to restrict the search space and avoid non-determinsm in Spacer
		x += f();
		assert(y > x);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (der:173-186): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
