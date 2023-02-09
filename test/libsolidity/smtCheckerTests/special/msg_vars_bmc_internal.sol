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

		assert(data.length == msg.data.length); // should fail with BMC
		assert(sender == msg.sender); // should fail with BMC
		assert(sig == msg.sig); // should fail with BMC
		assert(value == msg.value); // should fail with BMC
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (394-432): BMC: Assertion violation happens here.
// Warning 4661: (460-488): BMC: Assertion violation happens here.
// Warning 4661: (516-538): BMC: Assertion violation happens here.
// Warning 4661: (566-592): BMC: Assertion violation happens here.
// Info 6002: BMC: 8 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
