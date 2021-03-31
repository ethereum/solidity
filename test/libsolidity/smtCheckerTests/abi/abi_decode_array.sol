pragma abicoder v2;

contract C {
	function abiDecodeArray(bytes memory b1, bytes memory b2) public pure {
		(uint[] memory a, uint[] memory b) = abi.decode(b1, (uint[], uint[]));
		assert(a.length == b.length); // should fail

		(uint[] memory c, uint[] memory d) = abi.decode(b1, (uint[], uint[]));
		assert(a.length == c.length);
		assert(a.length == d.length); // should fail
		assert(b.length == d.length);
		assert(b.length == c.length); // should fail

		(uint[] memory e, uint[] memory f, uint[] memory g) = abi.decode(b1, (uint[], uint[], uint[]));
		assert(e.length == a.length); // should fail
		assert(f.length == b.length); // should fail
		assert(e.length == g.length); // should fail

		(uint[][] memory h, uint[][][] memory i, uint j) = abi.decode(b1, (uint[][], uint[][][], uint));
		require(j < h.length);
		require(j < i.length);
		require(j < i[j].length);
		assert(h[j].length == i[j][j].length); // should fail

		(uint[] memory k, uint[] memory l) = abi.decode(b2, (uint[], uint[]));
		assert(k.length == a.length); // should fail
		assert(k.length == l.length); // should fail
		assert(l.length == b.length); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8364: (162-168): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (170-176): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (283-289): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (291-297): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (532-538): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (540-546): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (548-554): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (769-775): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (769-777): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (779-785): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (779-787): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (779-789): Assertion checker does not yet implement type type(uint256[] memory[] memory[] memory)
// Warning 8364: (989-995): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (997-1003): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 6328: (182-210): CHC: Assertion violation happens here.
// Warning 6328: (335-363): CHC: Assertion violation happens here.
// Warning 6328: (414-442): CHC: Assertion violation happens here.
// Warning 6328: (560-588): CHC: Assertion violation happens here.
// Warning 6328: (607-635): CHC: Assertion violation happens here.
// Warning 6328: (654-682): CHC: Assertion violation happens here.
// Warning 6328: (879-916): CHC: Assertion violation happens here.
// Warning 6328: (1009-1037): CHC: Assertion violation happens here.
// Warning 6328: (1056-1084): CHC: Assertion violation happens here.
// Warning 6328: (1103-1131): CHC: Assertion violation happens here.
// Warning 8364: (162-168): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (170-176): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (283-289): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (291-297): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (532-538): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (540-546): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (548-554): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (769-775): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (769-777): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (779-785): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (779-787): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (779-789): Assertion checker does not yet implement type type(uint256[] memory[] memory[] memory)
// Warning 8364: (989-995): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (997-1003): Assertion checker does not yet implement type type(uint256[] memory)
