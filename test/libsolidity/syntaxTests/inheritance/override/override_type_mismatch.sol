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
}
contract X is A, B, C, D {
	struct MyStruct { int abc; }
	enum ENUM { F,G,H }

	int public override testvar;
	function test() internal override returns (uint256);
	function foo() internal override(MyStruct, ENUM, A, B, C, D) returns (uint256);
}
// ----
// TypeError: (0-132): Contract "A" should be marked as abstract.
// TypeError: (133-236): Contract "B" should be marked as abstract.
// TypeError: (237-295): Contract "C" should be marked as abstract.
// TypeError: (296-354): Contract "D" should be marked as abstract.
// TypeError: (355-600): Contract "X" should be marked as abstract.
