object "A" {
  code {
  }
  data "1" hex"0"
  data "1" hex"wronghexencoding"
}
// ----
// ParserError 2314: (37-41): Expected 'StringLiteral' but got 'ILLEGAL'
