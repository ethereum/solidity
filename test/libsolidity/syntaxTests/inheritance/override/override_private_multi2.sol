contract A {
	function test() private returns (uint256) {}
}
contract B {
	function test() public returns (uint256) {}
}

contract X is A, B {
}
// ----
// TypeError: (75-118): Overriding function visibility differs.
// TypeError: (75-118): Private functions cannot be overridden.
