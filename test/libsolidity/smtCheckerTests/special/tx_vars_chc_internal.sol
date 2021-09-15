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
	}
}
// ====
// SMTEngine: chc
// ----
