object "A" {
  code {
  }
  data "1" hex"wronghexencoding"
  data "2" hex"0"
}
// ----
// ParserError 2314: (37-41): Expected 'StringLiteral' but got 'ILLEGAL'
