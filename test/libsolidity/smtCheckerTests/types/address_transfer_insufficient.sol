contract C
{
	function f(address payable a, address payable b) public {
		require(a.balance == 0);
		a.transfer(600);
		b.transfer(1000);
		// Fails since a == this is possible.
		assert(a.balance == 600);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 8656: (101-116): CHC: Insufficient funds happens here.
// Warning 8656: (120-136): CHC: Insufficient funds happens here.
// Warning 6328: (180-204): CHC: Assertion violation happens here.
