contract C {
	address coin;
	uint dif;
	uint prevrandao;
	uint gas;
	uint number;
	uint timestamp;
	function f() public {
		coin = block.coinbase;
		dif = block.difficulty;
		prevrandao = block.prevrandao;
		gas = block.gaslimit;
		number = block.number;
		timestamp = block.timestamp;

		g();
	}
	function g() internal view {
		assert(uint160(coin) >= 0); // should hold
		assert(dif >= 0); // should hold
		assert(prevrandao > 2**64); // should hold
		assert(gas >= 0); // should hold
		assert(number >= 0); // should hold
		assert(timestamp >= 0); // should hold

		assert(coin == block.coinbase); // should fail with BMC
		assert(dif == block.difficulty); // should fail with BMC
		assert(prevrandao == block.prevrandao); // should fail with BMC
		assert(gas == block.gaslimit); // should fail with BMC
		assert(number == block.number); // should fail with BMC
		assert(timestamp == block.timestamp); // should fail with BMC
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 8417: (155-171): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 8417: (641-657): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 4661: (409-435): BMC: Assertion violation happens here.
// Warning 4661: (569-599): BMC: Assertion violation happens here.
// Warning 4661: (627-658): BMC: Assertion violation happens here.
// Warning 4661: (686-724): BMC: Assertion violation happens here.
// Warning 4661: (752-781): BMC: Assertion violation happens here.
// Warning 4661: (809-839): BMC: Assertion violation happens here.
// Warning 4661: (867-903): BMC: Assertion violation happens here.
