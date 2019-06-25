contract Errort6 {
  using foo for  ; // missing type name
}

// ----
// ParserError: (36-37): Expected type name
// Warning: (59-60): Recovered in ContractDefinition at '}'.
