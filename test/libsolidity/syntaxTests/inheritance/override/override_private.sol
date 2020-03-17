contract A {
	function test() private returns (uint256) {}
}
contract X is A {
	function test() private returns (uint256) {}
}
// ----
// TypeError: (80-124): Private functions cannot be overridden.
