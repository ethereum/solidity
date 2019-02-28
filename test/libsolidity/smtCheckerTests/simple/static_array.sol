pragma experimental SMTChecker;
contract C
{
	// Used to crash because Literal had no type
	int[3] d;
	// Used to crash because Literal had no type
	int[3*1] x;
}
// ----
// Warning: (92-100): Assertion checker does not yet support the type of this variable.
// Warning: (149-159): Assertion checker does not yet support the type of this variable.
// Warning: (153-156): Assertion checker does not yet implement this operator on non-integer types.
