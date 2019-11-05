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
	function test() internal override(B, D, C) returns (uint256);
	function foo() internal override(A, C) returns (uint256);
}
// ----
// TypeError: (528-545): Invalid contract specified in override list: C.
// TypeError: (590-604): Function needs to specify overridden contracts B and D.
