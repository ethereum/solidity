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
