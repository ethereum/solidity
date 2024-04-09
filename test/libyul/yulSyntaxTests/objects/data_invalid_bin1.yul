object "A" {
  code {
  }
  data "1" bin"1"
  data "1" bin"wrongbinencoding"
}
// ----
// ParserError 2314: (37-41): Expected 'StringLiteral' but got 'ILLEGAL'
