contract Errort6 {
  using foo for  ; // missing type name
}
// ----
// ParserError 3546: (36-37): Expected type name
// Warning 3796: (59-60): Recovered in ContractDefinition at '}'.
