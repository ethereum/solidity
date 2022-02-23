abstract contract A {
	function test() private virtual returns (uint256);
}
abstract contract X is A {
	function test() private override returns (uint256) {}
}
// ----
// TypeError 3942: (23-73): "virtual" and "private" cannot be used together.
