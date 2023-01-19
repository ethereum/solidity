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

		assert(bhash == blockhash(12));
		assert(coin == block.coinbase);
		assert(dif == block.difficulty);
		assert(prevrandao == block.prevrandao);
		assert(glimit == block.gaslimit);
		assert(number == block.number);
		assert(tstamp == block.timestamp);
		assert(mdata.length == msg.data.length);
		assert(sender == msg.sender);
		assert(sig == msg.sig);
		assert(value == msg.value);
		assert(gprice == tx.gasprice);
		assert(origin == tx.origin);
	}

	function fi() internal view {
		assert(bhash == blockhash(12));
		assert(coin == block.coinbase);
		assert(dif == block.difficulty);
		assert(prevrandao == block.prevrandao);
		assert(glimit == block.gaslimit);
		assert(number == block.number);
		assert(tstamp == block.timestamp);
		assert(mdata.length == msg.data.length);
		assert(sender == msg.sender);
		assert(sig == msg.sig);
		assert(value == msg.value);
		assert(gprice == tx.gasprice);
		assert(origin == tx.origin);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8417: (293-309): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 8417: (645-661): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 8417: (1127-1143): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
