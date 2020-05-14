contract C {
	function() internal view returns(uint256) immutable z;
	constructor() public {
		z = f;
	}
	function f() public view returns (uint256) {
		return 7;
	}
	function callZ() public view returns (uint) {
		return z();
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 7
// callZ() -> 7
