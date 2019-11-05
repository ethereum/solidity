interface A {
	function test() external returns (uint256);
	function test2() external returns (uint256);
}

interface B {
	function test() external returns (uint256);
	function test2() external returns (uint256);
}
abstract contract X is A, B {
	function test() external override(A, B) returns (uint256);
	function test2() external override(B, A) returns (uint256);
}
// ----
