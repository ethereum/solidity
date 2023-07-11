contract C {
	bytes32 bhash;
	address coin;
	uint dif;
	uint prevrandao;
	uint glimit;
	uint number;
	uint tstamp;
	bytes mdata;
	address sender;
	bytes4 sig;
	uint value;
	uint gprice;
	address origin;

	function f() public payable {
		bhash = blockhash(12);
		coin = block.coinbase;
		dif = block.difficulty;
		prevrandao = block.prevrandao;
		glimit = block.gaslimit;
		number = block.number;
		tstamp = block.timestamp;
		mdata = msg.data;
		sender = msg.sender;
		sig = msg.sig;
		value = msg.value;
		gprice = tx.gasprice;
		origin = tx.origin;

		fi();

		assert(bhash == blockhash(122));
		assert(coin != block.coinbase);
		assert(dif != block.difficulty);
		assert(prevrandao != block.prevrandao);
		assert(glimit != block.gaslimit);
		assert(number != block.number);
		assert(tstamp != block.timestamp);
		assert(mdata.length != msg.data.length);
		assert(sender != msg.sender);
		assert(sig != msg.sig);
		assert(value != msg.value);
		assert(gprice != tx.gasprice);
		assert(origin != tx.origin);
	}

	function fi() internal view {
		assert(bhash == blockhash(122));
		assert(coin != block.coinbase);
		assert(dif != block.difficulty);
		assert(prevrandao != block.prevrandao);
		assert(glimit != block.gaslimit);
		assert(number != block.number);
		assert(tstamp != block.timestamp);
		assert(mdata.length != msg.data.length);
		assert(sender != msg.sender);
		assert(sig != msg.sig);
		assert(value != msg.value);
		assert(gprice != tx.gasprice);
		assert(origin != tx.origin);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 8417: (293-309): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 8417: (646-662): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 8417: (1129-1145): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 6328: (563-594): CHC: Assertion violation happens here.
// Warning 6328: (598-628): CHC: Assertion violation happens here.
// Warning 6328: (632-663): CHC: Assertion violation happens here.
// Warning 6328: (667-705): CHC: Assertion violation happens here.
// Warning 6328: (709-741): CHC: Assertion violation happens here.
// Warning 6328: (745-775): CHC: Assertion violation happens here.
// Warning 6328: (779-812): CHC: Assertion violation happens here.
// Warning 6328: (816-855): CHC: Assertion violation happens here.
// Warning 6328: (859-887): CHC: Assertion violation happens here.
// Warning 6328: (891-913): CHC: Assertion violation happens here.
// Warning 6328: (917-943): CHC: Assertion violation happens here.
// Warning 6328: (947-976): CHC: Assertion violation happens here.
// Warning 6328: (980-1007): CHC: Assertion violation happens here.
// Warning 6328: (1046-1077): CHC: Assertion violation happens here.
// Warning 6328: (1081-1111): CHC: Assertion violation happens here.
// Warning 6328: (1115-1146): CHC: Assertion violation happens here.
// Warning 6328: (1150-1188): CHC: Assertion violation happens here.
// Warning 6328: (1192-1224): CHC: Assertion violation happens here.
// Warning 6328: (1228-1258): CHC: Assertion violation happens here.
// Warning 6328: (1262-1295): CHC: Assertion violation happens here.
// Warning 6328: (1299-1338): CHC: Assertion violation happens here.
// Warning 6328: (1342-1370): CHC: Assertion violation happens here.
// Warning 6328: (1374-1396): CHC: Assertion violation happens here.
// Warning 6328: (1400-1426): CHC: Assertion violation happens here.
// Warning 6328: (1430-1459): CHC: Assertion violation happens here.
// Warning 6328: (1463-1490): CHC: Assertion violation happens here.
