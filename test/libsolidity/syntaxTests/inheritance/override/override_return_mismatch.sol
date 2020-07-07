abstract contract A {
	function foo() internal virtual returns (uint256);
}
abstract contract B {
	function foo() internal virtual returns (uint8);
	function test() internal virtual returns (uint256);
}
abstract contract X is A, B {
	function test() internal override virtual returns (uint256);
}
// ----
// TypeError 6480: (203-296): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types.
