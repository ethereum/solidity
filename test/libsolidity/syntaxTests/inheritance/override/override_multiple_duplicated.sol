abstract contract A {
	function foo() internal virtual returns (uint256);
	function test(uint8 _a) internal virtual returns (uint256);
}
abstract contract B {
	function foo() internal virtual returns (uint256);
	function test() internal virtual returns (uint256);
}
abstract contract C {
	function foo() internal virtual returns (uint256);
}
abstract contract D {
	function foo() internal virtual returns (uint256);
	function test() internal virtual returns (uint256);
}
abstract contract X is A, B, C, D {
	function test() internal override(B, D, D) virtual returns (uint256);
	function foo() internal override(A, C, B, B, B, D ,D) virtual returns (uint256);
}
// ----
// TypeError 4520: (548-549): Duplicate contract "D" found in override list of "test".
// TypeError 4520: (621-622): Duplicate contract "B" found in override list of "foo".
// TypeError 4520: (624-625): Duplicate contract "B" found in override list of "foo".
// TypeError 4520: (630-631): Duplicate contract "D" found in override list of "foo".
