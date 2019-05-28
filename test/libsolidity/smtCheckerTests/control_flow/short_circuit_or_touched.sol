pragma experimental SMTChecker;

contract C
{
	bool b;
	function f() public {
		if ((b = true) || (b == false)) {}
		if ((b == true) || (b = false)) {}
		if ((b = true) || (b = false)) {}
		if ((b == true) || (b == false)) {}
		if ((b = false) || b) {}
	}
}
// ----
// Warning: (84-110): Condition is always true.
// Warning: (121-147): Condition is always true.
// Warning: (158-183): Condition is always true.
// Warning: (194-221): Condition is always true.
// Warning: (232-248): Condition is always false.
