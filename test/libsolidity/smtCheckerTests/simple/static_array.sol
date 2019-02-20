pragma experimental SMTChecker;
contract C
{
	// Used to crash because Literal had no type
	int[3] d;
	// Used to crash because Literal had no type
	int[3*1] x;
}
// ----
