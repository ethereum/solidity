pragma experimental SMTChecker;
contract C
{
	struct S {
		uint x;
	}
	mapping (uint => S) smap;
	function f(uint y, uint v) public {
		if (0==1)
			smap[y] = S(v);
	}
}
// ----
// Warning 6838: (140-144): Condition is always false.
// Warning 8364: (159-160): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 4639: (159-163): Assertion checker does not yet implement this expression.
