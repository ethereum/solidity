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
// Warning 1218: (544-563): CHC: Error trying to invoke SMT solver.
// Warning 1218: (567-586): CHC: Error trying to invoke SMT solver.
// Warning 1218: (590-609): CHC: Error trying to invoke SMT solver.
// Warning 1218: (613-632): CHC: Error trying to invoke SMT solver.
// Warning 6328: (544-563): CHC: Assertion violation might happen here.
// Warning 6328: (567-586): CHC: Assertion violation might happen here.
// Warning 6328: (590-609): CHC: Assertion violation might happen here.
// Warning 6328: (613-632): CHC: Assertion violation might happen here.
// Warning 4661: (544-563): BMC: Assertion violation happens here.
// Warning 4661: (567-586): BMC: Assertion violation happens here.
// Warning 4661: (590-609): BMC: Assertion violation happens here.
// Warning 4661: (613-632): BMC: Assertion violation happens here.
