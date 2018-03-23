interface I {
	function f() private;
}
// ----
// TypeError: Functions in interfaces cannot be internal or private.
