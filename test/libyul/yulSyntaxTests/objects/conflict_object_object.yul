object "A" {
  code {}

  object "B" {
    code {}
  }
  object "B" {
    code {}
  }
}
// ----
// ParserError 8794: (64-67): Object name "B" already exists inside the containing object.
