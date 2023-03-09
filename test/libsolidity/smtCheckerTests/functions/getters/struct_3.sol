contract C {
	struct S {
		string s;
		bytes b;
	}

	S public m;

	constructor() {
		m.s = "foo";
		m.b = "bar";
	}

	function f() public view {
		(string memory s, bytes memory b) = this.m();
		assert(keccak256(bytes(s)) == keccak256(bytes(m.s))); // should hold
		assert(b[0] == m.b[0]); // should hold
		assert(b[0] == "t"); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (307-326): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
