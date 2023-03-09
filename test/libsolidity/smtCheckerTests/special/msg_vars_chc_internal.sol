contract C {
	bytes data;
	address sender;
	bytes4 sig;
	uint value;
	function f() public payable {
		data = msg.data;
		sender = msg.sender;
		sig = msg.sig;
		value = msg.value;
		require(value == 42);

		g();
	}
	function g() internal view {
		assert(data.length >= 0); // should hold
		assert(uint160(sender) >= 0); // should hold
		assert(uint32(sig) >= 0); // should hold
		assert(value >= 0); // should hold

		assert(data.length == msg.data.length); // should hold with CHC
		assert(sender == msg.sender); // should hold with CHC
		assert(sig == msg.sig); // should hold with CHC
		assert(value == msg.value); // should hold with CHC

		assert(msg.value == 10); // should fail
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 6328: (645-668): CHC: Assertion violation happens here.
// Info 1391: CHC: 8 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
