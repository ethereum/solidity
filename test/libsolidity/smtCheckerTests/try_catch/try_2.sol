pragma experimental SMTChecker;
contract C {
	int x;

	function g() public  {
		x = 42;
	}

	function f() public {
		x = 0;
		try this.g() {
			assert(x == 42); // should hold
		} catch (bytes memory s) {
			assert(x == 0); // should hold
		}
	}
}
// ----
// Warning 5667: (187-201): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
