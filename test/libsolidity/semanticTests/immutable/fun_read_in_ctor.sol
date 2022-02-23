contract A {
	uint8 immutable a;
	uint8 x;

	constructor() {
		a = 3;
		x = readA();
	}

	function readX() public view returns (uint8) {
		return x;
	}

	function readA() public view returns (uint8) {
		return a;
	}
}
// ====
// compileViaYul: also
// ----
// readX() -> 3
// readA() -> 3
