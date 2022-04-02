contract C
{
	function f(uint x, address payable a, address payable b) public {
		require(a != b);
		require(x == 100);
		require(x == a.balance);
		require(a.balance == b.balance);
		a.transfer(600);
		b.transfer(100);
		// Fails since a == this is possible.
		assert(a.balance > b.balance);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (262-291='assert(a.balance > b.balance)'): CHC: Assertion violation happens here.
// Warning 1236: (184-199='a.transfer(600)'): BMC: Insufficient funds happens here.
// Warning 1236: (203-218='b.transfer(100)'): BMC: Insufficient funds happens here.
