struct S {
    uint x;
}

function suffix(S memory) pure returns (S memory) {}

contract C {
    uint x = (S suffix);
}
// ----
// ParserError 2314: (109-115): Expected ',' but got identifier
