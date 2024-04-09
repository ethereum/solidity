object "A" {
  code {
  }
  data "1" bin"wrongbinencoding"
  data "2" bin"1"
}
// ----
// ParserError 2314: (37-41): Expected 'StringLiteral' but got 'ILLEGAL'
