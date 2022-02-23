abstract contract A {
	function test() external virtual returns (uint256);
	function test2() external returns (uint256) {}
}
abstract contract X is A {
	function test() external returns (uint256) {}
	function test2() external override(A) returns (uint256) {}
}
// ----
// TypeError 9456: (153-198): Overriding function is missing "override" specifier.
// TypeError 4334: (76-122): Trying to override non-virtual function. Did you forget to add "virtual"?
