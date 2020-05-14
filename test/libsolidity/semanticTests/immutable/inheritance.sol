contract A {
	uint8 immutable a;
	constructor() public {
		a = 4;
	}
}
contract B is A {
	uint8 immutable b;
	constructor() public {
		b = 3;
	}
}
contract C is A {
	uint8 immutable c;
	constructor() public {
		c = 2;
	}
}
contract D is B, C {
	uint8 immutable d;

	constructor() public {
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
