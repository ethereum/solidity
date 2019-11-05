interface A {
	function test() external returns (uint256);
	function test2() external returns (uint256);
}
abstract contract X is A {
	function test() external override returns (uint256);
	function test2() external override(A) returns (uint256);
}
// ----
