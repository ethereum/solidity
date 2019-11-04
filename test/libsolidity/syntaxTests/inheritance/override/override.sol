contract A {
	int public testvar;
	function test() internal returns (uint256);
	function test2() internal returns (uint256);
}
contract X is A {
	int public override testvar;
	function test() internal override returns (uint256);
	function test2() internal override(A) returns (uint256);
}
// ----
// TypeError: (0-126): Contract "A" should be marked as abstract.
// TypeError: (127-288): Contract "X" should be marked as abstract.
