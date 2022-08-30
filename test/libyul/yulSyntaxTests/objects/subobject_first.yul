object "A" {
  object "B" {
    code {}
  }

  code {}
}
// ----
// ParserError 2314: (15-21): Expected 'code' but got identifier
