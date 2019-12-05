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
// TypeError: (203-296): Derived contract must override function "foo". Function with the same name and parameter types defined in two or more base classes.
