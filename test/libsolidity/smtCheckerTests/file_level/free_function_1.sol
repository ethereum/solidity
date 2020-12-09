pragma experimental SMTChecker;
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
// ----
// Warning 6660: (220-334): Model checker analysis was not possible because file level functions are not supported.
// Warning 6660: (220-334): Model checker analysis was not possible because file level functions are not supported.
