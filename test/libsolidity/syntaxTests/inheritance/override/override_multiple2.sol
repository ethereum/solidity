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
