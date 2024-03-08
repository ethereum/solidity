contract A {
	uint immutable a;
	constructor() {
		a = 7;
	}
	function f() public view returns (uint) { return a; }
}
contract B {
	uint immutable a;
	constructor() {
		a = 5;
	}
	function f() public view returns (uint) { return a; }
}
contract C {
	uint immutable a;
	uint public x;
	uint public y;
	constructor() {
		a = 3;
		x = (new A()).f();
		y = (new B()).f();
	}
	function f() public returns (uint256, uint, uint) {
		return (a, (new A()).f(), (new B()).f());
	}
}
// ----
// f() -> 3, 7, 5
// gas irOptimized: 86796
// gas irOptimized code: 37200
// gas legacy: 87728
// gas legacy code: 60800
// gas legacyOptimized: 86771
// gas legacyOptimized code: 37200
// x() -> 7
// y() -> 5
