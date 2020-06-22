abstract contract A {
	function foo() internal virtual returns (uint256);
	function test(uint8 _a) internal virtual returns (uint256);
}
abstract contract B {
	function foo() internal virtual returns (uint256);
	function test() internal virtual returns (uint256);
}
abstract contract C {
	function foo() internal virtual returns (uint256);
}
abstract contract D {
	function foo() internal virtual returns (uint256);
}
abstract contract X is A, B, C, D {
	struct MyStruct { int abc; }
	enum ENUM { F,G,H }

	function test() internal override virtual returns (uint256);
	function foo() internal override(MyStruct, ENUM, A, B, C, D) virtual returns (uint256);
}
// ----
// TypeError 9301: (602-610): Expected contract but got struct X.MyStruct.
// TypeError 9301: (612-616): Expected contract but got enum X.ENUM.
