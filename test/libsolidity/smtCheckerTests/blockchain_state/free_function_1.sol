function l(address payable a) {}

contract C {
	uint x;
	function f(address payable a) public payable {
		require(msg.value > 1);
		uint b1 = address(this).balance;
		l(a);
		uint b2 = address(this).balance;
		assert(b1 == b2); // should hold
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(x <= 0)\n
