abstract contract A {
	int public testvar;
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
	int public override testvar;
	function test() internal override(B, D, D) returns (uint256);
	function foo() internal override(A, C, B, B, B, D ,D) returns (uint256);
}
// ----
// TypeError: (599-600): Duplicate contract "D" found in override list of "test".
// TypeError: (664-665): Duplicate contract "B" found in override list of "foo".
// TypeError: (667-668): Duplicate contract "B" found in override list of "foo".
// TypeError: (673-674): Duplicate contract "D" found in override list of "foo".
