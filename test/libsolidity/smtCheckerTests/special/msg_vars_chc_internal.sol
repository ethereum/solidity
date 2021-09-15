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
	}
}
// ====
// SMTEngine: chc
// ----
