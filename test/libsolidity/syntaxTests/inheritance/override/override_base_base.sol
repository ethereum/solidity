contract A {
	function foo() internal returns (uint256);
}

contract B is A {
	function foo() internal override returns (uint256);
}

contract C is B {
	function foo() internal override returns (uint256);
}

contract D is C {
	function foo() internal override returns (uint256);
}

contract X is D {
	function foo() internal override returns (uint256);
}
// ----
// TypeError: (0-58): Contract "A" should be marked as abstract.
// TypeError: (60-132): Contract "B" should be marked as abstract.
// TypeError: (134-206): Contract "C" should be marked as abstract.
// TypeError: (208-280): Contract "D" should be marked as abstract.
// TypeError: (282-354): Contract "X" should be marked as abstract.
