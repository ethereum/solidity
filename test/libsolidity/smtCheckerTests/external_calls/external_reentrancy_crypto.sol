abstract contract D {
	function d() virtual public;
}

contract C {
	bytes data;

	bytes32 kec;

	constructor(bytes memory _data) {
		data = _data;

		kec = keccak256(data);
	}

	function check(bytes memory _data) public view {
		bytes32 _kec = keccak256(data);
		assert(_kec == kec); // should hold
		assert(kec == keccak256(_data)); // should fail
	}

	function ext(D d) public {
		d.d();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (302-333): CHC: Error trying to invoke SMT solver.
// Warning 6328: (302-333): CHC: Assertion violation might happen here.
// Warning 1218: (302-333): CHC: Error trying to invoke SMT solver.
// Warning 6328: (302-333): CHC: Assertion violation might happen here.
// Warning 0: (55-395): Contract invariants for :C:\n((!((kec + ((- 1) * keccak256(data))) >= 0) || ((kec' + ((- 1) * keccak256(data'))) >= 0)) && (!((kec + ((- 1) * keccak256(data))) <= 0) || ((kec' + ((- 1) * keccak256(data'))) <= 0)))\n((!(<errorCode> = 1) || !((kec + ((- 1) * keccak256(data))) = 0)) && (!((kec + ((- 1) * keccak256(data))) <= 0) || ((kec' + ((- 1) * keccak256(data'))) <= 0)) && (!((kec + ((- 1) * keccak256(data))) >= 0) || ((kec' + ((- 1) * keccak256(data'))) >= 0)))\n(((kec + ((- 1) * keccak256(data))) >= 0) && ((kec + ((- 1) * keccak256(data))) <= 0))\n
// Warning 4661: (302-333): BMC: Assertion violation happens here.
