pragma experimental SMTChecker;

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
// ----
// Warning: (136-155): Assertion violation happens here
// Warning: (229-234): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (300-302): Assertion checker does not yet implement type type(library l1)
// Warning: (229-234): Overflow (resulting value larger than 2**256 - 1) happens here
// Warning: (327-332): Overflow (resulting value larger than 2**256 - 1) happens here
