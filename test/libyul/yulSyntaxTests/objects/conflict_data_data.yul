object "A" {
  code {}

  data "B" ""
  data "B" hex"00"
}
// ----
// ParserError 8794: (45-48): Object name "B" already exists inside the containing object.
