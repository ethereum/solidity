pragma experimental "v0.5.0";
interface I {
	function f();
}
// ----
// SyntaxError: No visibility specified.
// TypeError: Functions in interfaces must be declared external.
