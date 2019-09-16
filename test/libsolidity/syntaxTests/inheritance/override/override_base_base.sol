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
