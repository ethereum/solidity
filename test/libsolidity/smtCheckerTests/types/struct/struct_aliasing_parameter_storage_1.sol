contract C {
    struct S {
        mapping(address => uint) innerM;
		uint sum;
    }

	function f(S storage m, address i, uint v) internal {
		m.innerM[i] = v;
		m.sum += v;
	}

	S s;

	constructor(uint amt) {
		f(s, msg.sender, amt);
	}
	function g() public view {
		assert(s.sum == 0); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (270-288): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
