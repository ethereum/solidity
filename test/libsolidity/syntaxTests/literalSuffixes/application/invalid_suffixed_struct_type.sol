struct S {
    uint x;
}

function suffix(S memory) pure suffix returns (S memory) {}

contract C {
    uint x = (S suffix);
}
// ----
// ParserError 2314: (116-122): Expected ',' but got identifier
