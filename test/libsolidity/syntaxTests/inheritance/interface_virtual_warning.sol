interface I {
	function foo() virtual external;
}
// ----
// Warning 5815: (15-47='function foo() virtual external;'): Interface functions are implicitly "virtual"
