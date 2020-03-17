contract A {
	function test() private returns (uint256) {}
}
contract X is A {
	function test() public returns (uint256) {}
}
// ----
// TypeError: (80-123): Overriding function visibility differs.
// TypeError: (80-123): Private functions cannot be overridden.
