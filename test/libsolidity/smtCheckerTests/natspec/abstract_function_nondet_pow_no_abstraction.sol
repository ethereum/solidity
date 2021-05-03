contract C {
	function e(uint _e) public pure {
		// Without abstracting function `pow` the solver
		// fails to prove that `++i` does not overflow.
		for (uint i = 0; i < _e; ++i)
			pow(_e, _e);
	}

	function pow(uint base, uint exponent) internal pure returns (uint) {
		if (base == 0) {
			return 0;
		}
		if (exponent == 0) {
			return 1;
		}
		if (exponent == 1) {
			return base;
		}
		uint y = 1;
		while(exponent > 1) {
			if(exponent % 2 == 0) {
				base = base * base;
				exponent = exponent / 2;
			} else {
				y = base * y;
				base = base * base;
				exponent = (exponent - 1) / 2;
			}
		}
		return base * y;
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 4984: (176-179): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4281: (435-447): CHC: Division by zero might happen here.
// Warning 4984: (467-478): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4281: (495-507): CHC: Division by zero might happen here.
// Warning 4984: (529-537): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (550-561): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 3944: (579-591): CHC: Underflow (resulting value less than 0) might happen here.
// Warning 4984: (616-624): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
