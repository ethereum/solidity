object "A" {
  code {}

  data "B" ""
  object "B" {
    code {}
  }
}
// ----
// ParserError 8794: (47-50): Object name "B" already exists inside the containing object.
