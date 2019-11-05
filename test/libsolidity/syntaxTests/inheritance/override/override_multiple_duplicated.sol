abstract contract A {
	int public testvar;
	function foo() internal returns (uint256);
	function test(uint8 _a) internal returns (uint256);
}
abstract contract B {
	function foo() internal returns (uint256);
	function test() internal returns (uint256);
}
abstract contract C {
	function foo() internal returns (uint256);
}
abstract contract D {
	function foo() internal returns (uint256);
	function test() internal returns (uint256);
}
abstract contract X is A, B, C, D {
	int public override testvar;
	function test() internal override(B, D, D) returns (uint256);
	function foo() internal override(A, C, B, B, B, D ,D) returns (uint256);
}
// ----
// TypeError: (543-544): Duplicate contract "D" found in override list of "test".
// TypeError: (608-609): Duplicate contract "B" found in override list of "foo".
// TypeError: (611-612): Duplicate contract "B" found in override list of "foo".
// TypeError: (617-618): Duplicate contract "D" found in override list of "foo".
