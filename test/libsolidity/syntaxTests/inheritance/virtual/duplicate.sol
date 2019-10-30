contract C
{
	function foo() virtual public virtual {}
	modifier modi() virtual virtual {_;}
	int virtual public virtual variable;
}
// ----
// ParserError: (44-51): Virtual already specified.
// ParserError: (80-87): Virtual already specified.
// ParserError: (113-120): Virtual already specified.
