function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = suffix 1000;
}
// ----
// ParserError 2314: (100-104): Expected ';' but got 'Number'
