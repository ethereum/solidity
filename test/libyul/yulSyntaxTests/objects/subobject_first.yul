object "A" {
  object "B" {
    code {}
  }

  code {}
}
// ----
// ParserError 4846: (15-21): Expected keyword "code".
