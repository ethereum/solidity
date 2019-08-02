pragma experimental SMTChecker;

contract C {
	struct S { uint x; }

	function g() internal pure returns (uint, S memory) {
		return (2, S(3));
	}
	function f(uint a) public pure {
		uint x;
		S memory y;
		if (a > 100)
			(x, y) = g();
	}
}
// ----
// Warning: (112-120): Assertion checker does not yet support the type of this variable.
// Warning: (137-141): Assertion checker does not yet implement type struct C.S memory
// Warning: (137-141): Assertion checker does not yet implement this expression.
// Warning: (193-203): Assertion checker does not yet support the type of this variable.
// Warning: (137-141): Assertion checker does not yet implement type struct C.S memory
// Warning: (137-141): Assertion checker does not yet implement this expression.
// Warning: (227-228): Assertion checker does not yet implement type struct C.S memory
