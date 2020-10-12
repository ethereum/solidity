pragma experimental SMTChecker;

contract C {
	address t;
	constructor() {
		t = address(this);
	}
	function f() public view {
		g(address(this));
	}
	function g(address a) internal view {
		assert(a == t);
		assert(a == address(this));
	}
}
