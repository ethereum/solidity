contract C {
	address coin;
	uint dif;
	uint gas;
	uint number;
	uint timestamp;
	function f() public {
		coin = block.coinbase;
		dif = block.difficulty;
		gas = block.gaslimit;
		number = block.number;
		timestamp = block.timestamp;

		g();
	}
	function g() internal view {
		assert(uint160(coin) >= 0); // should hold
		assert(dif >= 0); // should hold
		assert(gas >= 0); // should hold
		assert(number >= 0); // should hold
		assert(timestamp >= 0); // should hold

		assert(coin == block.coinbase); // should fail with BMC
		assert(dif == block.difficulty); // should fail with BMC
		assert(gas == block.gaslimit); // should fail with BMC
		assert(number == block.number); // should fail with BMC
		assert(timestamp == block.timestamp); // should fail with BMC
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (473-503): BMC: Assertion violation happens here.
// Warning 4661: (531-562): BMC: Assertion violation happens here.
// Warning 4661: (590-619): BMC: Assertion violation happens here.
// Warning 4661: (647-677): BMC: Assertion violation happens here.
// Warning 4661: (705-741): BMC: Assertion violation happens here.
