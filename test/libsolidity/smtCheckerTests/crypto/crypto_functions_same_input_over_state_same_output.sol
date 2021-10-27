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
// Info 1180: Contract invariant(s) for :C:\n(((erc + ((- 1) * ecrecover(tuple_constructor(h, v, r, s)))) <= 0) && ((erc + ((- 1) * ecrecover(tuple_constructor(h, v, r, s)))) >= 0))\n(((kec + ((- 1) * keccak256(data))) >= 0) && ((kec + ((- 1) * keccak256(data))) <= 0))\n(((rip + ((- 1) * ripemd160(data))) <= 0) && ((rip + ((- 1) * ripemd160(data))) >= 0))\n(((sha + ((- 1) * sha256(data))) <= 0) && ((sha + ((- 1) * sha256(data))) >= 0))\n
