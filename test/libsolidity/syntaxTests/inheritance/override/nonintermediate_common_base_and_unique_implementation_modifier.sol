contract I {
	modifier f() virtual { _; }
}
contract J {
	modifier f() virtual { _; }
}
contract IJ is I, J {
	modifier f() virtual override (I, J) { _; }
}
contract A is IJ
{
	modifier f() override { _; }
}
contract B is IJ
{
}
contract C is A, B {}
// ----
// TypeError 6480: (229-250): Derived contract must override modifier "f". Two or more base classes define modifier with same name.
