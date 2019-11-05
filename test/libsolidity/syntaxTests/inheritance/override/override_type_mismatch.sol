abstract contract A {
	int public testvar;
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

	int public override testvar;
	function test() internal override returns (uint256);
	function foo() internal override(MyStruct, ENUM, A, B, C, D) returns (uint256);
}
// ----
// TypeError: (645-653): Expected contract but got struct X.MyStruct.
// TypeError: (655-659): Expected contract but got enum X.ENUM.
