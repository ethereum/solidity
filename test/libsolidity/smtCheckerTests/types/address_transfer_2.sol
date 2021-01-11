pragma experimental SMTChecker;

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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (295-324): CHC: Assertion violation happens here.
// Warning 1236: (217-232): BMC: Insufficient funds happens here.
// Warning 1236: (236-251): BMC: Insufficient funds happens here.
