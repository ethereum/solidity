pragma experimental SMTChecker;

contract C {
	function c() public pure returns (uint) { return 42; }
}

contract B is C {
	function b() public pure returns (uint) { return c(); }
}

contract A is B {
	uint public x;

	function a() public {
		x = b();
		assert(x < 40);
	}
}
// ----
// Warning 6328: (254-268): Assertion violation happens here
