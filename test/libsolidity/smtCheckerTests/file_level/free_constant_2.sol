uint256 constant x = 56;
enum ActionChoices {GoLeft, GoRight, GoStraight, Sit}
ActionChoices constant choices = ActionChoices.GoRight;
bytes32 constant st = "abc\x00\xff__";
contract C {
	function i() public returns (uint, ActionChoices, bytes32) {
		return (x, choices, st);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2018: (188-278): Function state mutability can be restricted to pure
// Warning 8195: (0-23): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (79-133): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (135-172): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (0-23): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (79-133): Model checker analysis was not possible because file level constants are not supported.
// Warning 8195: (135-172): Model checker analysis was not possible because file level constants are not supported.
