contract C {
	function o() public pure {
		uint x = type(uint).max;
		unchecked { ++x; }
		assert(x == type(uint).min);
	}

	function u() public pure {
		uint x = type(uint).min;
		unchecked { --x; }
		assert(x == type(uint).max);
	}

	function o_int() public pure {
		int x = type(int).max;
		unchecked { ++x; }
		assert(x == type(int).min);
	}

	function u_int() public pure {
		int x = type(int).min;
		unchecked { --x; }
		assert(x == type(int).max);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
