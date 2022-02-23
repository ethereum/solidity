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
// ====
// SMTEngine: all
// ----
// Warning 6838: (108-112): BMC: Condition is always false.
