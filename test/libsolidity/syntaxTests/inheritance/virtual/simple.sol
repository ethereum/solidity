contract C
{
	function foo() public virtual {}
	function foo2() virtual public {}
	modifier modi() virtual {_;}
	int public virtual variable;
	int virtual public variable2;
}
