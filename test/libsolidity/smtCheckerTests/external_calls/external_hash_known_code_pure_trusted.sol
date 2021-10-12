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
// SMTContract: C
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Info 1180: Contract invariant(s) for :C:\n((sig_1 <= 0) && (sig_2 <= 0))\nReentrancy property(ies) for :Crypto:\n(<errorCode> = 0)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(prevOwner == owner)\n<errorCode> = 3 -> Assertion failed at assert(sig_1 == sig_2)\n
