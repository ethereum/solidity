contract c {
	uint[] a;
	function f() returns (uint) {
		a = [,2,3];
		return (a[0]);
	}
}
// ----
// ParserError 4799: (62-63): Expected expression (inline array elements cannot be omitted).
