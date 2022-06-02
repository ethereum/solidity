enum E {A, B, C}

function suffix(E) pure returns (E) {}

contract C {
    E e = E.A suffix;
}
// ----
// ParserError 2314: (85-91): Expected ';' but got identifier
