contract D {
}

contract C {
	struct S {
		D d;
		function () external returns (uint) f;
	}

	S public s;

	function test() public view {
		(D d, function () external returns (uint) f) = this.s();
		assert(d == s.d); // should hold
		assert(address(d) == address(this)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (146-183): Unused local variable.
// Warning 8364: (187-193): Assertion checker does not yet implement type function () view external returns (contract D,function () external returns (uint256))
// Warning 6328: (234-269): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
