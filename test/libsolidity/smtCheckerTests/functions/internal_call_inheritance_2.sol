pragma experimental SMTChecker;

contract C {
	uint y;
	function c(uint _y) public returns (uint) {
		y = _y;
		return y;
	}
}

contract B is C {
	function b() public returns (uint) { return c(42); }
}

contract A is B {
	uint public x;

	function a() public {
		x = b();
		assert(x < 40);
	}
}
// ----
// Warning 6328: (274-288): Assertion violation happens here
