contract C {
	function f(uint x) public pure returns (uint) {
		return 2 * x;
	}
	function g() public view returns (function (uint) external returns (uint)) {
		return this.f;
	}
	function h(uint x) public returns (uint) {
		return this.g()(x) + 1;
	}
	function t() external view returns (
			function(uint) external returns (uint) a,
			function(uint) external view returns (uint) b) {
		a = this.f;
		b = this.f;
	}
}
// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f(uint256): 2 -> 4
// h(uint256): 2 -> 5
// t() -> 0x86e73fe93c23a38c4bdba9e4b9cc0ddcc0fe293eb3de648b0000000000000000, 0x86e73fe93c23a38c4bdba9e4b9cc0ddcc0fe293eb3de648b0000000000000000

