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
// Warning 6660: (188-302): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (188-302): Model checker analysis was not possible because file level functions are not supported.
