pragma experimental SMTChecker;
uint constant A = 42;
contract C {
	uint[] data;
	function f(uint x, uint[] calldata input) public view returns (uint, uint) {
		(uint a, uint[] calldata b) = fun(input, data);
		return (a, b.length + x + A);
	}
}
function fun(uint[] calldata _x, uint[] storage _y) view  returns (uint, uint[] calldata) {
	return (_y[0], _x);
}
// ----
// Warning 8195: (32-52): Model checker analysis was not possible because file level constants are not supported.
// Warning 6660: (246-360): Model checker analysis was not possible because file level functions are not supported.
// Warning 8195: (32-52): Model checker analysis was not possible because file level constants are not supported.
// Warning 6660: (246-360): Model checker analysis was not possible because file level functions are not supported.
