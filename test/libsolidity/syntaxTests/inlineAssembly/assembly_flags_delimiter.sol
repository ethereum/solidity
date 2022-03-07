function f() pure {
	assembly ("a" "b") {}
}
// ----
// ParserError 2314: (35-38): Expected ')' but got 'StringLiteral'
