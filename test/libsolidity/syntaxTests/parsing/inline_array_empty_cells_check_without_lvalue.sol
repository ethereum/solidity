contract c {
	uint[] a;
	function f() returns (uint, uint) {
		return ([3, ,4][0]);
	}
}
// ----
// ParserError: (75-75): Expected expression (inline array elements cannot be omitted).
