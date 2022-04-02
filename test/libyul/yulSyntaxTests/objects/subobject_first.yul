object "A" {
  object "B" {
    code {}
  }

  code {}
}
// ----
// ParserError 4846: (15-21='object'): Expected keyword "code".
