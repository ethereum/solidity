contract C {
	function g() public pure returns (uint) {
	}
	function f() public view {
		uint choice = 42;
		try this.g() returns (uint) {
			choice = 1;
		} catch{
			choice = 10;
			try this.g() returns (uint) {
				choice = 2;
			} catch {
				choice = 3;
			}
		}
		assert(choice >= 1 && choice <= 3); // should hold
		assert(choice == 42); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (323-343): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
