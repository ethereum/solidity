contract A {
	function test() public returns (uint256) {}
}
contract B {
	function test() private returns (uint256) {}
}

contract X is A, B {
}
// ----
// TypeError: (74-118): Overriding function visibility differs.
