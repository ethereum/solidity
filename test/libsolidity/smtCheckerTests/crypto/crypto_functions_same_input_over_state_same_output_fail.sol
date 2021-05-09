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
// ====
// SMTEngine: all
// ----
// Warning 1218: (693-712): CHC: Error trying to invoke SMT solver.
// Warning 6328: (693-712): CHC: Assertion violation might happen here.
// Warning 1218: (716-735): CHC: Error trying to invoke SMT solver.
// Warning 6328: (716-735): CHC: Assertion violation might happen here.
// Warning 1218: (739-758): CHC: Error trying to invoke SMT solver.
// Warning 6328: (739-758): CHC: Assertion violation might happen here.
// Warning 1218: (762-781): CHC: Error trying to invoke SMT solver.
// Warning 6328: (762-781): CHC: Assertion violation might happen here.
// Warning 4661: (693-712): BMC: Assertion violation happens here.
// Warning 4661: (716-735): BMC: Assertion violation happens here.
// Warning 4661: (739-758): BMC: Assertion violation happens here.
// Warning 4661: (762-781): BMC: Assertion violation happens here.
