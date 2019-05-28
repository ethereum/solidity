pragma experimental SMTChecker;

contract C
{
	bool b;
	function f() public {
		if ((b = false) && (b == true)) {}
		if ((b == false) && (b = true)) {}
		if ((b = false) && (b = true)) {}
		if ((b == false) && (b == true)) {}
		if ((b = true) && b) {}
	}
}
// ----
// Warning: (84-110): Condition is always false.
// Warning: (121-147): Condition is always true.
// Warning: (158-183): Condition is always false.
// Warning: (194-221): Condition is always false.
// Warning: (232-247): Condition is always true.
