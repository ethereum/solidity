object "A" {
  code {}

  object "A" {
    code {}
  }
}
// ----
// ParserError 8311: (33-36): Object name cannot be the same as the name of the containing object.
