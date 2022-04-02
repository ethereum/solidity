contract C {
  constructor() payable payable {}
}
// ----
// ParserError 9680: (37-44='payable'): State mutability already specified as "payable".
