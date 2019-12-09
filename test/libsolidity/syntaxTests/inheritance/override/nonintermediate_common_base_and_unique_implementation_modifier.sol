contract I {
	modifier f() { _; }
}
contract J {
	modifier f() { _; }
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
// TypeError: (14-33): Trying to override non-virtual modifier. Did you forget to add "virtual"?
// TypeError: (50-69): Trying to override non-virtual modifier. Did you forget to add "virtual"?
