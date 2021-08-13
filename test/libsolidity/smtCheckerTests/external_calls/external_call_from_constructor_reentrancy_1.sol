interface D {
	function ext(C c) external returns (uint);
}

contract C {
	uint x;
	function s(uint _x) public { x = _x; }
	constructor(D d) {
		uint a = d.ext(this);
		assert(x == 0); // should hold because there's no reentrancy from the constructor
		assert(a == 2); // should fail
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (253-267): CHC: Assertion violation happens here.
