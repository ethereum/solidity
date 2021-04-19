contract C {
	uint[] data;
	function f(uint x, uint[] calldata input) public view returns (uint, uint) {
		(uint a, uint[] calldata b) = fun(input, data);
		return (a, b.length + x);
	}
}
function fun(uint[] calldata _x, uint[] storage _y) view  returns (uint, uint[] calldata) {
	return (_y[0], _x);
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (168-180): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\ndata = []\nx = 1\n = 0\n = 0\na = 0\n\nTransaction trace:\nC.constructor()\nState: data = []\nC.f(1, input)\n    fun(_x, []) -- internal call
// Warning 6368: (289-294): CHC: Out of bounds access happens here.\nCounterexample:\ndata = []\nx = 0\ninput = [5, 5, 5, 5, 5, 5, 5, 5, 5, 10, 5, 5, 13, 5, 5, 5, 5, 5, 19, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5]\n = 0\n = 0\n\nTransaction trace:\nC.constructor()\nState: data = []\nC.f(0, [5, 5, 5, 5, 5, 5, 5, 5, 5, 10, 5, 5, 13, 5, 5, 5, 5, 5, 19, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5])\n    fun([5, 5, 5, 5, 5, 5, 5, 5, 5, 10, 5, 5, 13, 5, 5, 5, 5, 5, 19, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5], []) -- internal call
