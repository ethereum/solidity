pragma experimental SMTChecker;

contract C
{
	mapping (string => uint) map;
	function f(string memory s, uint x) public {
		map[s] = x;
		assert(x == map[s]);
	}
}
// ----
// Warning: (89-104): Assertion checker does not yet support the type of this variable.
// Warning: (129-130): Internal error: Expression undefined for SMT solver.
// Warning: (129-130): Assertion checker does not yet implement this type.
// Warning: (155-156): Internal error: Expression undefined for SMT solver.
// Warning: (155-156): Assertion checker does not yet implement this type.
// Warning: (139-158): Assertion violation happens here
