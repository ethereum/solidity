{
    function f() -> x {}
    let 123 := f()
}
// ----
// ParserError 2314: (35-38): Expected identifier but got 'Number'
