contract C {
	uint[] public array;

	function f() public {
		// (f()) is not a tuple expression, but its value is a tuple.
		(f()) = ();
	}

	function g() public {
		// (revert()) is not a tuple expression, but its value is a tuple.
		(revert()) = ();
	}

	function h() internal returns (uint, uint) {}

	function i() public {
		// (h()) is not a tuple expression, but its value is a tuple (uint, uint).
		(h()) = (1, 1);
	}

	function j() public returns (uint, uint) {
		// (j()) is not a tuple expression, but its value is a tuple (uint, uint).
		(j()) = (1, 1);
	}

	function m() public {
		// (uint x, uint y) is a tuple expression, and its value is a tuple (uint, uint).
		(uint x, uint y) = (1, 1);
	}

	function n() public {
		// ((array.push(), array.push())) is not a tuple expression, but contains a tuple expression, and the value of both is a tuple (pointer uint, pointer uint).
		((array.push(), array.push())) = (1, 1);
	}
}
// ----
// TypeError 4247: (126-129): Expression has to be an lvalue.
// TypeError 4247: (236-244): Expression has to be an lvalue.
// TypeError 4247: (407-410): Expression has to be an lvalue.
// TypeError 4247: (550-553): Expression has to be an lvalue.
