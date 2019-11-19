interface I {
	function foo() virtual external;
}
// ----
// Warning: (15-47): Interface functions are implicitly "virtual"
