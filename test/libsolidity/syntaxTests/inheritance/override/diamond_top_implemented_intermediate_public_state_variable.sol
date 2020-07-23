contract I {
	function f() external view virtual returns (uint) { return 1; }
}
contract A is I
{
	uint public override f;
}
contract B is I
{
}
contract C is A, B {}
// ----
// TypeError 6480: (145-166): Derived contract must override function "f". Two or more base classes define function with same name and parameter types. Since one of the bases defines a public state variable which cannot be overridden, you have to change the inheritance layout or the names of the functions.
