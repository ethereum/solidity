contract C
{
	function f(address a, address b) public view {
		uint x = b.balance + 1000 ether;
		assert(a.balance > b.balance);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (63-69): Unused local variable.
// Warning 4984: (72-94): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (98-127): CHC: Assertion violation happens here.
