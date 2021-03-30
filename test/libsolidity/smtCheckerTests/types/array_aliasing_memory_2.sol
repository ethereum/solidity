pragma experimental SMTChecker;
pragma abicoder               v2;

contract C
{
	uint[] array;
	function p() public {
		array.push();
	}
	function f(uint[] memory a, uint[] memory b) public {
		require(a.length > 0);
		require(b.length > 0);
		require(array.length > 0);
		array[0] = 42;
		a[0] = 2;
		// Access is safe but oob is reported due of aliasing.
		b[0] = 1;
		// Erasing knowledge about memory references should not
		// erase knowledge about state variables.
		// Removed because current Spacer seg faults.
		//assert(array[0] == 42);
		// Accesses are safe but oob is reported due of aliasing.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (359-363): CHC: Out of bounds access happens here.
// Warning 6368: (616-620): CHC: Out of bounds access happens here.
// Warning 6328: (609-626): CHC: Assertion violation happens here.
// Warning 6368: (637-641): CHC: Out of bounds access happens here.
