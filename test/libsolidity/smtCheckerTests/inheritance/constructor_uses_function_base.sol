contract A {
	uint x;
	constructor() {
		x = 42;
	}
	function f() public view returns(uint256) {
		return x;
	}
}
contract B is A {
	uint y = f();
}
contract C is B {
	function g() public view {
		assert(y == 42);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(!(y <= 41) && !(y >= 43))\n
