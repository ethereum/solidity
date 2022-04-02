interface I {
	function f() internal;
}
// ----
// TypeError 1560: (15-37='function f() internal;'): Functions in interfaces must be declared external.
