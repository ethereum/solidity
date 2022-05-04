contract C {
    struct S {
        mapping(address => uint) innerM;
		uint sum;
    }

	struct T {
		uint x;
		S s;
	}

	function f(T storage m, address i, uint v) internal {
		m.s.innerM[i] = v;
		m.s.sum += v;
	}

	T t;

	constructor(uint amt) {
		f(t, msg.sender, amt);
	}
	function g() public view {
		assert(t.s.sum == 0); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (307-327): CHC: Assertion violation happens here.
