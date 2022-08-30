contract C {
	uint256 immutable x;
	uint256 immutable y;
	constructor() {
		x = 42;
		y = 23;
	}
	function f() public view returns (uint256, uint256) {
		return (x+x,y);
	}
}
// ----
// f() -> 84, 23
