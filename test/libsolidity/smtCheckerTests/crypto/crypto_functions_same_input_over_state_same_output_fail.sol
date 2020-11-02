pragma experimental SMTChecker;

contract C {
	bytes data;
	bytes32 h;
	uint8 v;
	bytes32 r;
	bytes32 s;

	bytes32 kec;
	bytes32 sha;
	bytes32 rip;
	address erc;

	constructor(bytes memory _data, bytes32 _h, uint8 _v, bytes32 _r, bytes32 _s) {
		data = _data;
		h = _h;
		v = _v;
		r = _r;
		s = _s;

		kec = keccak256(data);
		sha = sha256(data);
		rip = ripemd160(data);
		erc = ecrecover(h, v, r, s);
	}

	function set(bytes memory _data, bytes32 _h, uint8 _v, bytes32 _r, bytes32 _s) public {
		data = _data;
		h = _h;
		v = _v;
		r = _r;
		s = _s;
	}

	function f() public view {
		bytes32 _kec = keccak256(data);
		bytes32 _sha = sha256(data);
		bytes32 _rip = ripemd160(data);
		address _erc = ecrecover(h, v, r, s);
		assert(_kec == kec);
		assert(_sha == sha);
		assert(_rip == rip);
		assert(_erc == erc);
	}
}
// ----
// Warning 1218: (726-745): CHC: Error trying to invoke SMT solver.
// Warning 6328: (726-745): CHC: Assertion violation might happen here.
// Warning 1218: (749-768): CHC: Error trying to invoke SMT solver.
// Warning 6328: (749-768): CHC: Assertion violation might happen here.
// Warning 1218: (772-791): CHC: Error trying to invoke SMT solver.
// Warning 6328: (772-791): CHC: Assertion violation might happen here.
// Warning 6328: (795-814): CHC: Assertion violation happens here.
// Warning 4661: (726-745): BMC: Assertion violation happens here.
// Warning 4661: (749-768): BMC: Assertion violation happens here.
// Warning 4661: (772-791): BMC: Assertion violation happens here.
