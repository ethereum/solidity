contract A
{
	function foo() virtual internal {}
}
contract B
{
	function foo() internal {}
}
contract C is A, B
{
	function foo() internal override(A, B) {}
}
// ----
// TypeError: (65-91): Trying to override non-virtual function. Did you forget to add "virtual"?
