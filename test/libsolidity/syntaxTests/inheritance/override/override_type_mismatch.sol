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
// TypeError: (552-560): Expected contract but got struct X.MyStruct.
// TypeError: (562-566): Expected contract but got enum X.ENUM.
