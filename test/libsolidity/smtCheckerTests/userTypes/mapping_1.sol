type MyInt is int;
contract C {
    mapping(MyInt => int) m;
	function f(MyInt a) public view {
		assert(m[a] == 0); // should hold
		assert(m[a] != 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreInv: yes
// ----
// Warning 6328: (134-151): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
