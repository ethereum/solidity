contract C
{
	uint[] array;
	constructor() {
		array.push();
		array.push();
		array.push();
		array.push();
	}
	function f(uint x, uint y) public {
		require(x < array.length);
		array[x] = 200;
		require(x == y);
		assert(array[y] > 100);
	}
}
// ====
// SMTEngine: all
// ----
