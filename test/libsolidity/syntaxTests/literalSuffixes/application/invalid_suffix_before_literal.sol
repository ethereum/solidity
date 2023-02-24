function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = suffix 1000;
}
// ----
// ParserError 2314: (107-111): Expected ';' but got 'Number'
