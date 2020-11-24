pragma experimental SMTChecker;
pragma abicoder               v2;

contract C
{
	uint[] array;
	function f(uint[] memory a, uint[] memory b) public {
		array[0] = 42;
		a[0] = 2;
		b[0] = 1;
		// Erasing knowledge about memory references should not
		// erase knowledge about state variables.
		// Removed because current Spacer seg faults.
		//assert(array[0] == 42);
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning 6328: (371-388): CHC: Assertion violation happens here.
