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
// Warning: (140-144): Condition is always false.
// Warning: (149-156): Assertion checker does not yet implement type struct C.S storage ref
// Warning: (159-160): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning: (159-163): Assertion checker does not yet implement type struct C.S memory
// Warning: (159-163): Assertion checker does not yet implement this expression.
// Warning: (149-163): Assertion checker does not yet implement type struct C.S storage ref
