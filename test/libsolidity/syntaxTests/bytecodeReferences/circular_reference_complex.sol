contract D {}
contract C is D {}
contract E is D
{
	function foo() public { new C(); }
}
// ----
