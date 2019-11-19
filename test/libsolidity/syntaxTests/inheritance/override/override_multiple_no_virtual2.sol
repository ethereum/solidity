contract A
{
	function foo() virtual internal {}
}
contract B
{
	function foo() internal {}
}
contract C is A, B
{
}
// ----
// TypeError: (94-116): Derived contract must override function "foo". Function with the same name and parameter types defined in two or more base classes.
