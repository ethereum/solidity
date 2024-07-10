pragma abicoder v2;

contract C {
	function abiDecodeArray(bytes memory b1, bytes memory b2) public pure {
		(uint[] memory a, uint[] memory b) = abi.decode(b1, (uint[], uint[]));
		assert(a.length == b.length); // should fail

		(uint[] memory c, uint[] memory d) = abi.decode(b1, (uint[], uint[]));
		assert(a.length == c.length);
		assert(b.length == d.length);

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
		assert(l.length == b.length); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (182-210): CHC: Assertion violation happens here.\nCounterexample:\n\nc = []\nd = []\ne = []\nf = []\ng = []\nh = []\ni = []\nj = 0\nk = []\nl = []\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (466-494): CHC: Assertion violation happens here.\nCounterexample:\n\nf = [83, 83, 83, 83, 83, 83, 67, 83, 83, 83, 83, 83, 70, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 72, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 71, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83]\ng = [65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 55, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 60, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65]\nh = []\ni = []\nj = 0\nk = []\nl = []\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (513-541): CHC: Assertion violation happens here.\nCounterexample:\n\ng = [65, 65, 65, 65, 65, 65, 65, 64, 65, 47, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 48, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65]\nh = []\ni = []\nj = 0\nk = []\nl = []\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (560-588): CHC: Assertion violation happens here.\nCounterexample:\n\nb = []\nd = []\nh = []\ni = []\nj = 0\nk = []\nl = []\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (785-822): CHC: Assertion violation happens here.\nCounterexample:\n\nf = [144]\nj = 8365\nk = []\nl = []\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (915-943): CHC: Assertion violation happens here.\nCounterexample:\n\nj = 8365\nl = [35, 35, 35, 35, 35]\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (962-990): CHC: Assertion violation happens here.\nCounterexample:\n\nj = 32285\n\nTransaction trace:\nC.constructor()\nC.abiDecodeArray(b1, b2) -- counterexample incomplete; parameter name used instead of value
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
