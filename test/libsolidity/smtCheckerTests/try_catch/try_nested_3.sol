pragma experimental SMTChecker;
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
// ----
// Warning 6328: (355-375): CHC: Assertion violation happens here.\nCounterexample:\n\nchoice = 3\n\nTransaction trace:\nC.constructor()\nC.f()
