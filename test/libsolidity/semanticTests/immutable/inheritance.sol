contract A {
	uint8 immutable a;
	constructor() {
		a = 4;
	}
}
contract B is A {
	uint8 immutable b;
	constructor() {
		b = 3;
	}
}
contract C is A {
	uint8 immutable c;
	constructor() {
		c = 2;
	}
}
contract D is B, C {
	uint8 immutable d;

	constructor() {
		d = 1;
	}
	function f() public view returns (uint256, uint256, uint, uint) {
		return (a, b, c, d);
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 4, 3, 2, 1
