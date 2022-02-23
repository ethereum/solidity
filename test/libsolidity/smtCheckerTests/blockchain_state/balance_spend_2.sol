contract C {
	constructor() payable {
		require(msg.value > 100);
	}
	function f(address payable _a, uint _v) public {
		require(_v < 10);
		_a.transfer(_v);
	}
	function inv() public view {
		assert(address(this).balance > 0); // should fail
		assert(address(this).balance > 80); // should fail
		assert(address(this).balance > 90); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (193-226): CHC: Assertion violation might happen here.
// Warning 6328: (245-279): CHC: Assertion violation happens here.
// Warning 6328: (298-332): CHC: Assertion violation happens here.
// Warning 1236: (141-156): BMC: Insufficient funds happens here.
// Warning 4661: (193-226): BMC: Assertion violation happens here.
