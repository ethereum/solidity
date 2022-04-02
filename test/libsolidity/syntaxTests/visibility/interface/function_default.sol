interface I {
	function f();
}
// ----
// SyntaxError 4937: (15-28='function f();'): No visibility specified. Did you intend to add "external"?
// TypeError 1560: (15-28='function f();'): Functions in interfaces must be declared external.
