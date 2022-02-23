interface I {
}
contract A is I
{
	uint public f;
}
abstract contract B is I
{
	function f() external virtual returns (uint);
}
abstract contract C is A, B {}
// ----
// TypeError 6480: (128-158): Derived contract must override function "f". Two or more base classes define function with same name and parameter types. Since one of the bases defines a public state variable which cannot be overridden, you have to change the inheritance layout or the names of the functions.
