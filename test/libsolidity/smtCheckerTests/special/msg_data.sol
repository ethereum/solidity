pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(msg.data.length > 0);
	}
}
// ----
// Warning: (86-101): Assertion checker does not yet support this expression.
// Warning: (86-94): Assertion checker does not yet support this special variable.
// Warning: (86-94): Assertion checker does not yet implement this type.
// Warning: (86-101): Internal error: Expression undefined for SMT solver.
// Warning: (79-106): Assertion violation happens here
