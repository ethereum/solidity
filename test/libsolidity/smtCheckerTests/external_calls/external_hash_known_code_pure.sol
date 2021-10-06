contract Crypto {
	function hash(bytes32) external pure returns (bytes32) {
		return bytes32(0);
	}
}

contract C {
	address owner;
	bytes32 sig_1;
	bytes32 sig_2;
	Crypto d;

	constructor() {
		owner = msg.sender;
	}

	function f1(bytes32 _msg) public {
		address prevOwner = owner;
		sig_1 = d.hash(_msg);
		sig_2 = d.hash(_msg);
		assert(prevOwner == owner);
	}

	function inv() public view {
		assert(sig_1 == sig_2);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (398-420): CHC: Assertion violation happens here.
// Info 1180: Reentrancy property(ies) for :C:\n!(<errorCode> = 1)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(prevOwner == owner)\n<errorCode> = 3 -> Assertion failed at assert(sig_1 == sig_2)\n
