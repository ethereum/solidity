interface I {
	function f() external returns (uint);
}
contract A is I
{
	uint public override f;
}
abstract contract B is I
{
	function f() external virtual override returns (uint) { return 2; }
}
abstract contract C is A, B {}
// ----
// TypeError 6480: (198-228): Derived contract must override function "f". Two or more base classes define function with same name and parameter types. Since one of the bases defines a public state variable which cannot be overridden, you have to change the inheritance layout or the names of the functions.
