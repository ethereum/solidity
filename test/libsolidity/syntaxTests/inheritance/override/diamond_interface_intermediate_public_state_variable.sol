interface I {
	function f() external returns (uint);
}
abstract contract A is I
{
	uint public f;
}
abstract contract B is I
{
}
// This is fine because `f` is not implemented in `I` and `A.f` is the only mention below `I`.
abstract contract C is A, B {}
// ----
