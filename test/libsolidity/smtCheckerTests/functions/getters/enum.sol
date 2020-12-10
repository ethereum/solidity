pragma experimental SMTChecker;


contract C {
	enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
	ActionChoices public choice;

	function f() public view {
		ActionChoices e = this.choice();
		assert(e == choice); // should hold
		assert(e == ActionChoices.SitStill); // should fail
	}
}
// ----
// Warning 6328: (243-278): CHC: Assertion violation happens here.\nCounterexample:\nchoice = 0\n\n\n\nTransaction trace:\nconstructor()\nState: choice = 0\nf()
