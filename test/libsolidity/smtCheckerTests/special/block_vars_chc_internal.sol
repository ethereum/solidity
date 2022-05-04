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

		assert(coin == address(this)); // should fail
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 6328: (770-799): CHC: Assertion violation happens here.\nCounterexample:\ncoin = 0x1e28, dif = 0, gas = 0, number = 0, timestamp = 0\n\nTransaction trace:\nC.constructor()\nState: coin = 0x0, dif = 0, gas = 0, number = 0, timestamp = 0\nC.f(){ block.coinbase: 0x1e28, block.difficulty: 0, block.gaslimit: 0, block.number: 0, block.timestamp: 0 }\n    C.g() -- internal call
