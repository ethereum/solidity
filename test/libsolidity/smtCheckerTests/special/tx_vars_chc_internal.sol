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

		assert(gas == tx.gasprice); // should hold with CHC
		assert(origin == tx.origin); // should hold with CHC

		assert(tx.origin == address(this)); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreOS: macos
// ----
// Warning 6328: (343-377): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
