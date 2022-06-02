function suffix(uint value) pure returns (uint) { return value; }
function metasuffix(function (uint) pure returns (uint) value) pure returns (uint) { return value; }

contract C {
    uint x = suffix metasuffix;
}
// ----
// ParserError 2314: (201-211): Expected ';' but got identifier
