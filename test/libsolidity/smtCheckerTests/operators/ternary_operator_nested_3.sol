contract C {

	function increment(uint x) private pure returns (uint) {
		return x + 1;
	}

	function increment2(uint x) private pure returns (uint) {
		return x + 1;
	}

	function f(uint x) public pure returns (uint) {
		return x < 10 ? (x > 0 ? 0 : increment(x)) : (x > 100 ? increment2(x) : 0);
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 4984: (160-165): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
