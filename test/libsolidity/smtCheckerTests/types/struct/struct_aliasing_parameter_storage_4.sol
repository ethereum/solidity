contract C {
    struct S {
        mapping(address => uint) innerM;
		uint sum;
    }

	struct T {
		uint x;
		S s;
	}

	function f(S storage m, address i, uint v) internal {
		m.innerM[i] = v;
		m.sum += v;
	}

	T t;

	constructor(uint amt) {
		f(t.s, msg.sender, amt);
	}
	function g() public view {
		assert(t.s.sum == 0); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (305-325): CHC: Assertion violation happens here.
