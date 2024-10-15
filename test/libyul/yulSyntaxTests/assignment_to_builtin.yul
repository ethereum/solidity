{
	function f() -> x {}
    let add := f()
}
// ----
// ParserError 5568: (32-35): Cannot use builtin function name "add" as identifier name.
