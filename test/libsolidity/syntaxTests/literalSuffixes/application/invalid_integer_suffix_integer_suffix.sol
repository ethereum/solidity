function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 1000 suffix 1000 suffix;
}
// ----
// ParserError 2314: (105-109): Expected ';' but got 'Number'
