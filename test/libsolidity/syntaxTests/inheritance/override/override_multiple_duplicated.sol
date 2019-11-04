contract A {
	int public testvar;
	function foo() internal returns (uint256);
	function test(uint8 _a) internal returns (uint256);
}
contract B {
	function foo() internal returns (uint256);
	function test() internal returns (uint256);
}
contract C {
	function foo() internal returns (uint256);
}
contract D {
	function foo() internal returns (uint256);
	function test() internal returns (uint256);
}
contract X is A, B, C, D {
	int public override testvar;
	function test() internal override(B, D, D) returns (uint256);
	function foo() internal override(A, C, B, B, B, D ,D) returns (uint256);
}
// ----
// TypeError: (0-132): Contract "A" should be marked as abstract.
// TypeError: (133-236): Contract "B" should be marked as abstract.
// TypeError: (237-295): Contract "C" should be marked as abstract.
// TypeError: (296-399): Contract "D" should be marked as abstract.
// TypeError: (498-499): Duplicate contract "D" found in override list of "test".
// TypeError: (563-564): Duplicate contract "B" found in override list of "foo".
// TypeError: (566-567): Duplicate contract "B" found in override list of "foo".
// TypeError: (572-573): Duplicate contract "D" found in override list of "foo".
// TypeError: (400-595): Contract "X" should be marked as abstract.
