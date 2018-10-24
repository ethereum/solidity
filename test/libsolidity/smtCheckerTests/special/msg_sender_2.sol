pragma experimental SMTChecker;

contract C
{
	function f() public view {
		require(msg.sender != address(0));
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
	}
}
// ----
// Warning: (98-108): Assertion checker does not yet implement this expression.
// Warning: (98-108): Internal error: Expression undefined for SMT solver.
