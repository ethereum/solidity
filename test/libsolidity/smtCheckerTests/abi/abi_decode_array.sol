pragma experimental SMTChecker;
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
// ----
// Warning 8364: (194-200): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (202-208): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (315-321): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (323-329): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (564-570): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (572-578): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (580-586): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (801-807): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (801-809): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (811-817): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (811-819): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (811-821): Assertion checker does not yet implement type type(uint256[] memory[] memory[] memory)
// Warning 8364: (1021-1027): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (1029-1035): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 6328: (214-242): CHC: Assertion violation happens here.
// Warning 6328: (367-395): CHC: Assertion violation happens here.
// Warning 6328: (446-474): CHC: Assertion violation happens here.
// Warning 6328: (592-620): CHC: Assertion violation happens here.
// Warning 6328: (639-667): CHC: Assertion violation happens here.
// Warning 6328: (686-714): CHC: Assertion violation happens here.
// Warning 6328: (911-948): CHC: Assertion violation happens here.
// Warning 6328: (1041-1069): CHC: Assertion violation happens here.
// Warning 6328: (1088-1116): CHC: Assertion violation happens here.
// Warning 6328: (1135-1163): CHC: Assertion violation happens here.
// Warning 8364: (194-200): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (202-208): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (315-321): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (323-329): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (564-570): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (572-578): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (580-586): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (801-807): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (801-809): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (811-817): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (811-819): Assertion checker does not yet implement type type(uint256[] memory[] memory)
// Warning 8364: (811-821): Assertion checker does not yet implement type type(uint256[] memory[] memory[] memory)
// Warning 8364: (1021-1027): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (1029-1035): Assertion checker does not yet implement type type(uint256[] memory)
