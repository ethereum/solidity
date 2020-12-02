pragma experimental SMTChecker;

contract C {
	bytes32 bhash;
	address coin;
	uint dif;
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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (545-576): CHC: Assertion violation happens here.
// Warning 6328: (580-610): CHC: Assertion violation happens here.
// Warning 6328: (614-645): CHC: Assertion violation happens here.
// Warning 6328: (649-681): CHC: Assertion violation happens here.
// Warning 6328: (685-715): CHC: Assertion violation happens here.
// Warning 6328: (719-752): CHC: Assertion violation happens here.
// Warning 6328: (756-795): CHC: Assertion violation happens here.
// Warning 6328: (799-827): CHC: Assertion violation happens here.
// Warning 6328: (831-853): CHC: Assertion violation happens here.
// Warning 6328: (857-883): CHC: Assertion violation happens here.
// Warning 6328: (887-916): CHC: Assertion violation happens here.
// Warning 6328: (920-947): CHC: Assertion violation happens here.
// Warning 6328: (986-1017): CHC: Assertion violation happens here.
// Warning 6328: (1021-1051): CHC: Assertion violation happens here.
// Warning 6328: (1055-1086): CHC: Assertion violation happens here.
// Warning 6328: (1090-1122): CHC: Assertion violation happens here.
// Warning 6328: (1126-1156): CHC: Assertion violation happens here.
// Warning 6328: (1160-1193): CHC: Assertion violation happens here.
// Warning 6328: (1197-1236): CHC: Assertion violation happens here.
// Warning 6328: (1240-1268): CHC: Assertion violation happens here.
// Warning 6328: (1272-1294): CHC: Assertion violation happens here.
// Warning 6328: (1298-1324): CHC: Assertion violation happens here.
// Warning 6328: (1328-1357): CHC: Assertion violation happens here.
// Warning 6328: (1361-1388): CHC: Assertion violation happens here.
