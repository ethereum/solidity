contract C {
  modifier someModifier() { _; }
}

function fun() C.someModifier {

}
// ----
// ParserError 2314: (65-66): Expected '{' but got '.'
