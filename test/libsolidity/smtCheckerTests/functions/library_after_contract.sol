pragma experimental SMTChecker;

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

// ----
// Warning 2018: (131-190): Function state mutability can be restricted to pure
