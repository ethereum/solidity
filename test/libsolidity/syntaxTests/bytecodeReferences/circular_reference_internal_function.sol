contract C
{
	// Internal uncalled function should not cause an cyclic dep. error
	function foo() internal { new D(); }
	function callFoo() virtual public { foo(); }
}

contract D is C
{
	function callFoo() override public {}
}
// ----
