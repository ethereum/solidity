pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(msg.sig == 0x00000000);
	}
}
// ----
// Warning: (86-93): Assertion checker does not yet support this special variable.
// Warning: (86-93): Assertion checker does not yet implement this type.
// Warning: (86-107): Assertion checker does not yet implement the type bytes4 for comparisons
// Warning: (86-107): Internal error: Expression undefined for SMT solver.
// Warning: (79-108): Assertion violation happens here
