contract C {
	function g(uint y) public {
		uint z = L.f(y);
		assert(z == y);
	}
}

library L {
	function f(uint x) internal returns (uint) {
		return x;
	}
}

// ====
// SMTEngine: all
// ----
// Warning 2018: (98-157): Function state mutability can be restricted to pure
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
