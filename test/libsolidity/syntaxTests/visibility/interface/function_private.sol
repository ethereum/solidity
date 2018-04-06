interface I {
	function f() private;
}
// ----
// TypeError: (15-36): Functions in interfaces cannot be internal or private.
