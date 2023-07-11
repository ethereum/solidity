pragma abicoder               v2;

contract C
{
	uint[] array;
	constructor() {
		array.push();
	}
	function f(uint[] memory a, uint[] memory b) public {
		require(a.length > 0);
		array[0] = 42;
		uint[] storage c = array;
		a[0] = 2;
		require(b.length > 0);
		b[0] = 1;
		// Erasing knowledge about memory references should not
		// erase knowledge about state variables.
		assert(array[0] == 42);
		// Erasing knowledge about memory references should not
		// erase knowledge about storage references.
		assert(c[0] == 42);
		// Removed because current Spacer seg faults in cex generation.
		//assert(a[0] == 2);
		// Removed because current Spacer seg faults in cex generation.
		//assert(b[0] == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 7 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
