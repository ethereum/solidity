pragma experimental "v0.5.0";
contract C {
	function g() public pure returns (
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint,
		uint
	) { }
	function f() public pure returns (uint, uint, bytes32) {
		uint a;
		uint b;
		(,,,,a,,,,b,,,,) = g();
	}
}
// ----
