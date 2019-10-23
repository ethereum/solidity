contract A {
	int public testvar;
	function foo() internal returns (uint256);
	function test(uint8 _a) internal returns (uint256);
}
contract B {
	function foo() internal returns (uint256);
}

contract C is A {
}
contract D is A, B, C {
	function foo() internal override(A, B) returns (uint256);
}
// ----
// TypeError: (0-132): Contract "A" should be marked as abstract.
// TypeError: (133-191): Contract "B" should be marked as abstract.
// TypeError: (193-212): Contract "C" should be marked as abstract.
// TypeError: (213-297): Contract "D" should be marked as abstract.
