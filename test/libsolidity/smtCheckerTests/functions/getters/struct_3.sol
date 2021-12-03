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
// Warning 1218: (273-277): CHC: Error trying to invoke SMT solver.
// Warning 1218: (281-287): CHC: Error trying to invoke SMT solver.
// Warning 1218: (314-318): CHC: Error trying to invoke SMT solver.
// Warning 1218: (307-326): CHC: Error trying to invoke SMT solver.
// Warning 6368: (273-277): CHC: Out of bounds access might happen here.
// Warning 6368: (281-287): CHC: Out of bounds access might happen here.
// Warning 6368: (314-318): CHC: Out of bounds access might happen here.
// Warning 6328: (307-326): CHC: Assertion violation might happen here.
// Warning 4661: (307-326): BMC: Assertion violation happens here.
