contract C
{
	function f(uint x) public payable {
		assert(blockhash(x) > 0);
		assert(blockhash(2) > 0);
		uint y = x;
		assert(blockhash(x) == blockhash(y));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (52-76): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 38\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f(38){ value: 8365 }
// Warning 6328: (80-104): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 38\ny = 0\n\nTransaction trace:\nC.constructor()\nC.f(38){ value: 32285 }
