// Tests that two private functions cannot be defined with the same name.
contract A {
	function foo() private {}
	function foo() private {}
}
// ----
// DeclarationError 1686: (88-113): Function with same name and parameter types defined twice.
