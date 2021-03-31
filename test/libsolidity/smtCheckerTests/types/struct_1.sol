contract C
{
	struct S {
		uint x;
	}

	mapping (uint => S) smap;

	function f(uint y, uint v) public {
		smap[y] = S(v);
		S memory smem = S(v);
		assert(smap[y].x == smem.x);
	}
}
// ====
// SMTEngine: all
// ----
