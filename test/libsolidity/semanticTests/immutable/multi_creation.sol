contract A {
	uint immutable a;
	constructor() public {
		a = 7;
	}
	function f() public view returns (uint) { return a; }
}
contract B {
	uint immutable a;
	constructor() public {
		a = 5;
	}
	function f() public view returns (uint) { return a; }
}
contract C {
	uint immutable a;
	uint public x;
	uint public y;
	constructor() public {
		a = 3;
		x = (new A()).f();
		y = (new B()).f();
	}
	function f() public returns (uint256, uint, uint) {
		return (a, (new A()).f(), (new B()).f());
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 3, 7, 5
// x() -> 7
// y() -> 5
