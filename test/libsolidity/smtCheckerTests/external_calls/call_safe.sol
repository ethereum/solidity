contract C {
	uint x;
	function f(address a) public {
		(bool s, bytes memory data) = a.call("");
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreInv: yes
// ----
// Warning 2072: (57-63): Unused local variable.
// Warning 2072: (65-82): Unused local variable.
