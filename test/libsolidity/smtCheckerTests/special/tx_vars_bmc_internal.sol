contract C {
	uint gas;
	address origin;
	function f() public {
		gas = tx.gasprice;
		origin = tx.origin;

		g();
	}
	function g() internal view {
		assert(gas >= 0); // should hold
		assert(uint160(origin) >= 0); // should hold

		assert(gas == tx.gasprice); // should fail with BMC
		assert(origin == tx.origin); // should fail with BMC
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (233-259='assert(gas == tx.gasprice)'): BMC: Assertion violation happens here.
// Warning 4661: (287-314='assert(origin == tx.origin)'): BMC: Assertion violation happens here.
