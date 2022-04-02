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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (52-76='assert(blockhash(x) > 0)'): CHC: Assertion violation happens here.
// Warning 6328: (80-104='assert(blockhash(2) > 0)'): CHC: Assertion violation happens here.
