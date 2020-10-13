contract C {
  modifier someModifier() { _; }
}

function fun() C.someModifier {

}
// ----
// SyntaxError 5811: (49-83): Free functions cannot have modifiers.
