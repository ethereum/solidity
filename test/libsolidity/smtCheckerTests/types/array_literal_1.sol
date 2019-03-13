pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint[3] memory array = [uint(1), 2, 3];
	}
}
// ----
// Warning: (76-96): Unused local variable.
// Warning: (99-114): Assertion checker does not yet implement tuples and inline arrays.
// Warning: (99-114): Internal error: Expression undefined for SMT solver.
