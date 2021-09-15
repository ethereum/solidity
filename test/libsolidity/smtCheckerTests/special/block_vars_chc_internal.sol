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

		assert(coin == block.coinbase); // should hold with CHC
		assert(dif == block.difficulty); // should hold with CHC
		assert(gas == block.gaslimit); // should hold with CHC
		assert(number == block.number); // should hold with CHC
		assert(timestamp == block.timestamp); // should hold with CHC
	}
}
// ====
// SMTEngine: chc
// ----
