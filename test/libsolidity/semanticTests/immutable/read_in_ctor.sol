contract A {
	uint8 immutable a;
	uint8 x;

	constructor() {
		a = 3;
		x = a;
	}

	function readX() public view returns (uint8) {
		return x;
	}
}
// ====
// compileViaYul: also
// ----
// readX() -> 3
