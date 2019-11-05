abstract contract A {
	int public testvar;
	function foo() internal returns (uint256);
	function test(uint8 _a) internal returns (uint256);
}
abstract contract B {
	function foo() internal returns (uint256);
}

abstract contract C is A {
}
abstract contract D is A, B, C {
	function foo() internal override(A, B) returns (uint256);
}
// ----
