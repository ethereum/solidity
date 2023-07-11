using {f} for S global global;
struct S { uint x; }
function f(S memory _x) pure returns (uint) { return _x.x; }
// ----
// ParserError 2314: (23-29): Expected ';' but got identifier
