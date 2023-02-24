function hex(bytes memory) pure suffix returns (bytes memory) {}

contract C {
    bytes b = hex"1234"hex;
}
// ----
// ParserError 2314: (9-12): Expected identifier but got 'ILLEGAL'
