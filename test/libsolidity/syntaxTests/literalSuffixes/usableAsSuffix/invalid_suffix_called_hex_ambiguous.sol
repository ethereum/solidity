function hex(uint) pure suffix returns (bytes memory) {}

contract C {
    bytes b = 1 hex"1234";
}
// ----
// ParserError 2314: (9-12): Expected identifier but got 'ILLEGAL'
