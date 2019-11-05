abstract contract A {
	function foo() internal returns (uint256);
}

abstract contract B is A {
	function foo() internal override returns (uint256);
}

abstract contract C is B {
	function foo() internal override returns (uint256);
}

abstract contract D is C {
	function foo() internal override returns (uint256);
}

abstract contract X is D {
	function foo() internal override returns (uint256);
}
// ----
