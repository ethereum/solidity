contract D {
	uint x;
}

contract C {
	function f() public {
		D d1 = new D();
		D d2 = new D();

		assert(d1 != d2); // should hold in ext calls trusted mode
		assert(address(this) != address(d1)); // should hold in ext calls trusted mode
		assert(address(this) != address(d2)); // should hold in ext calls trusted mode
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Info 1180: Contract invariant(s) for :C:\n(:var 0).isActive[address(this)]\n
