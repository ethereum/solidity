pragma abicoder v2;

contract C {
	function f(bytes memory data) public pure {
		(uint x1, bool b1) = abi.decode(data, (uint, bool));
		(uint x2, bool b2) = abi.decode(data, (uint, bool));
		assert(x1 == x2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (91-98='bool b1'): Unused local variable.
// Warning 2072: (146-153='bool b2'): Unused local variable.
