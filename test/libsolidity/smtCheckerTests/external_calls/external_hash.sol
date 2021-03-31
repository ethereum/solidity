abstract contract Crypto {
	function hash(bytes32) external pure virtual returns (bytes32);
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
// Warning 6328: (390-412): CHC: Assertion violation happens here.
