abstract contract A {
	function foo() internal virtual returns (uint256);
}
abstract contract B {
	function foo() internal virtual returns (uint256);
	function test() internal virtual returns (uint256);
}
abstract contract X is A, B {
	function test() internal override returns (uint256) {}
}
// ----
// TypeError 6480: (205-292): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types.
