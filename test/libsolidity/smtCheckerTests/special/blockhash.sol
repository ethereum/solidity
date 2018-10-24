pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(blockhash(2) > 0);
	}
}
// ----
// Warning: (86-98): Assertion checker does not yet support this special variable.
// Warning: (86-98): Assertion checker does not yet implement this type.
// Warning: (86-102): Assertion checker does not yet implement the type bytes32 for comparisons
// Warning: (86-102): Internal error: Expression undefined for SMT solver.
// Warning: (79-103): Assertion violation happens here
