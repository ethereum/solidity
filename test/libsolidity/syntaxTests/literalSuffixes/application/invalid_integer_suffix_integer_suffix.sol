function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 1000 suffix 1000 suffix;
}
// ----
// ParserError 2314: (112-116): Expected ';' but got 'Number'
