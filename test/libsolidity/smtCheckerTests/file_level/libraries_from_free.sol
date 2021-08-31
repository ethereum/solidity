library L {
	function pub() public pure returns (uint) {
		return 7;
	}
	function inter() internal pure returns (uint) {
		return 8;
	}
}

function fu() pure returns (uint, uint) {
	return (L.pub(), L.inter());
}

contract C {
	function f() public pure {
		(uint x, uint y) = fu();
		assert(x == 7); // should hold but SMTChecker doesn't implement delegatecall
		assert(y == 9); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4588: (190-197): Assertion checker does not yet implement this type of function call.
// Warning 6328: (284-298): CHC: Assertion violation happens here.
// Warning 6328: (363-377): CHC: Assertion violation happens here.
