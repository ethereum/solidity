{
    switch codesize()
    case hex"00" {}
    case hex"1122" {}
}
// ----
// ParserError 3772: (33-40): Hex literals are not valid in this context.
