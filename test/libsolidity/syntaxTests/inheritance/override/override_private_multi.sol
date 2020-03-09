contract A {
	function test() private returns (uint256) {}
}
contract B {
	function test() private returns (uint256) {}
}

contract X is A, B {
}
// ----
// TypeError: (75-119): Private functions cannot be overridden.
