contract C {
	int public x;

	function f() public view {
		int y = 42;
		bool success = false;
		try this.x() returns (int v) {
			y = v;
			try this.x() returns (int w) {
				success = true;
				y = w;
			}
			catch {}
		} catch {}
		assert(!success || y == x); // should hold
		assert(y == 42); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (280-295): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
