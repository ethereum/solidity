pragma experimental SMTChecker;
contract C
{
	// Used to crash because Literal had no type
	int[3] d;
}
// ----
// Warning: (92-100): Assertion checker does not yet support the type of this variable.
