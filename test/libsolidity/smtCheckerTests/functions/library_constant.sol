library l1 {

	uint private constant TON = 1000;
	function f1() public pure {
		assert(TON == 1000);
		assert(TON == 2000);
	}
	function f2(uint x, uint y) internal pure returns (uint) {
		return x + y;
	}
}

contract C {
	function f(uint x) public pure {
		uint z = l1.f2(x, 1);
		assert(z == x + 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (103-122): CHC: Assertion violation happens here.
// Warning 4984: (196-201): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
