contract A {
	function test() public returns (uint256) {}
}
contract X is A {
	function test() private returns (uint256) {}
}
// ----
// TypeError: (79-123): Overriding function visibility differs.
