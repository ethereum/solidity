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
// SMTIgnoreInv: yes
// ----
// Warning 6328: (302-333): CHC: Assertion violation happens here.\nCounterexample:\nkec = 0x0\n_kec = 0x0\n\nTransaction trace:\nC.constructor(_data)\nState: kec = 0x0\nC.check(_data)
