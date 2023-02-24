function suffix(uint value) pure suffix returns (uint) { return value; }
function metasuffix(function (uint) pure returns (uint) value) pure suffix returns (uint) { return value; }

contract C {
    uint x = suffix metasuffix;
}
// ----
// ParserError 2314: (215-225): Expected ';' but got identifier
