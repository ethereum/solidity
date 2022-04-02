interface I {
	function f() private;
}
// ----
// TypeError 1560: (15-36='function f() private;'): Functions in interfaces must be declared external.
