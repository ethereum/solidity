contract I {
	function f() external view virtual returns (uint) { return 1; }
}
contract A is I
{
}
contract B is I
{
}
contract C is A, B
{
	uint public override f;
}
// ----
