contract C {
	uint x;
	uint y;

	function g(uint _x) public {
		uint z = f1(_x);
		assert(x == 0); // should hold because f1 is pure
		assert(z == _x); // should hold but f1 was abstracted as nondet, so it fails

		uint t = f2(_x);
		assert(y == 0); // should hold because f1 is pure and f2 is view
		assert(t == _x); // should hold
	}

	/// @custom:smtchecker abstract-function-nondet
	function f1(uint _x) internal pure returns (uint) {
		return _x;
	}

	function f2(uint _y) internal view returns (uint) {
		return _y;
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// ----
// Warning 2018: (33-335): Function state mutability can be restricted to view
// Warning 2018: (457-524): Function state mutability can be restricted to pure
// Warning 6328: (135-150): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
