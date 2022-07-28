contract A {
	function foo() private {}
	function foo() private {}
}
// ----
// DeclarationError 1686: (14-39): Function with same name and parameter types defined twice.
