
contract C {
	enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
	ActionChoices public choice;

	function f() public view {
		ActionChoices e = this.choice();
		assert(e == choice); // should hold
		assert(e == ActionChoices.SitStill); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (210-245): CHC: Assertion violation happens here.\nCounterexample:\nchoice = 0\ne = 0\n\nTransaction trace:\nC.constructor()\nState: choice = 0\nC.f()
