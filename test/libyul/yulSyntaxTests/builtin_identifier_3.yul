{
	function f(x) { f(add) }
}
// ----
// ParserError 7104: (21-24='add'): Builtin function "add" must be called.
