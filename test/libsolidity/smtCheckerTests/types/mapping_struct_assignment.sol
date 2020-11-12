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
// Warning 6838: (140-144): BMC: Condition is always false.
