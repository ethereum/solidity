pragma experimental SMTChecker;

contract C
{
	function f(bytes memory b) public pure returns (bytes memory) {
		bytes memory c = b;
		return b;
	}
}
// ----
// Warning: (113-127): Unused local variable.
// Warning: (113-127): Assertion checker does not yet support the type of this variable.
// Warning: (58-72): Assertion checker does not yet support the type of this variable.
// Warning: (95-107): Assertion checker does not yet support the type of this variable.
// Warning: (130-131): Internal error: Expression undefined for SMT solver.
// Warning: (130-131): Assertion checker does not yet implement this type.
