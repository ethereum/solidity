pragma experimental SMTChecker;

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
// ----
// Warning 1218: (335-366): CHC: Error trying to invoke SMT solver.
// Warning 6328: (335-366): CHC: Assertion violation might happen here.
// Warning 1218: (335-366): CHC: Error trying to invoke SMT solver.
// Warning 6328: (335-366): CHC: Assertion violation might happen here.
// Warning 4661: (335-366): BMC: Assertion violation happens here.
