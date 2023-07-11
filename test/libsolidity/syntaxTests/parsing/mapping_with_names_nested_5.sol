contract test {
    mapping(address owner => mapping(address spender => bytes32 note));
}
// ----
// ParserError 2314: (86-87): Expected identifier but got ';'
