pragma experimental SMTChecker;

contract C
{
	struct S {
		uint x;
	}

	mapping (uint => S) smap;

	function f(uint y, uint v) public {
		smap[y] = S(v);
		S memory smem = S(v);
	}
}
// ----
// Warning 2072: (157-170): Unused local variable.
// Warning 8364: (149-150): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 4639: (149-153): Assertion checker does not yet implement this expression.
// Warning 8364: (173-174): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 4639: (173-177): Assertion checker does not yet implement this expression.
