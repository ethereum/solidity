contract C
{
	function f(uint x) public pure {
		require(x < 100);
		uint y = 100;
		y += y + x;
		assert(y < 300);
		assert(y < 110);
	}
}
// ====
// SMTEngine: all
// SMTShowProvedSafe: yes
// ----
// Warning 6328: (118-133): CHC: Assertion violation happens here.
// Info 9576: (90-95): CHC: Overflow (resulting value larger than 2**256 - 1) check is safe!
// Info 9576: (85-95): CHC: Overflow (resulting value larger than 2**256 - 1) check is safe!
// Info 9576: (99-114): CHC: Assertion violation check is safe!
