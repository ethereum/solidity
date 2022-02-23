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
// Warning 6328: (645-668): CHC: Assertion violation happens here.\nCounterexample:\ndata = [0x26, 0x12, 0x1f, 0xf0], sender = 0x0, sig = 0x26121ff0, value = 42\n\nTransaction trace:\nC.constructor()\nState: data = [], sender = 0x0, sig = 0x0, value = 0\nC.f(){ msg.data: [0x26, 0x12, 0x1f, 0xf0], msg.sender: 0x0, msg.sig: 0x26121ff0, msg.value: 42 }\n    C.g() -- internal call
