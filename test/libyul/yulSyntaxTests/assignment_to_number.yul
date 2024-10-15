{
	function f() -> x {}
    let 123 := f()
}
// ----
// ParserError 2314: (32-35): Expected identifier but got 'Number'
