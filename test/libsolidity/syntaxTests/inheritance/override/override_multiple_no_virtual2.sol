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
// TypeError 6480: (94-116): Derived contract must override function "foo". Two or more base classes define function with same name and parameter types.
