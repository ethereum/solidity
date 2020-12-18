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

	function f() public view {
		bytes32 _kec = keccak256(data);
		assert(_kec == kec);
	}

	function ext(D d) public {
		d.d();
	}
}
