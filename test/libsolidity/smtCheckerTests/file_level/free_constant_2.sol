pragma experimental SMTChecker;
uint256 constant x = 56;
enum ActionChoices {GoLeft, GoRight, GoStraight, Sit}
ActionChoices constant choices = ActionChoices.GoRight;
bytes32 constant st = "abc\x00\xff__";
contract C {
	function i() public returns (uint, ActionChoices, bytes32) {
		return (x, choices, st);
	}
}
// ----
// Warning 2018: (220-310): Function state mutability can be restricted to pure
// Warning 8195: (32-55): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (111-165): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (167-204): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (32-55): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (111-165): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (167-204): Model checker analysis was not possible because file level constants are not supported.
