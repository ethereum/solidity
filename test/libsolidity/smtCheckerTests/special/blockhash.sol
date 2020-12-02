pragma experimental SMTChecker;

contract C
{
	function f(uint x) public payable {
		assert(blockhash(x) > 0);
		assert(blockhash(2) > 0);
		uint y = x;
		assert(blockhash(x) == blockhash(y));
	}
}
// ----
// Warning 6328: (85-109): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 7719\n\n\nTransaction trace:\nconstructor()\nf(7719)
// Warning 6328: (113-137): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 7719\n\n\nTransaction trace:\nconstructor()\nf(7719)
