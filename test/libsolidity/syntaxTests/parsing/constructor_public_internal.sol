contract C {
  constructor() public internal {}
}
// ----
// ParserError 9439: (36-44='internal'): Visibility already specified as "public".
