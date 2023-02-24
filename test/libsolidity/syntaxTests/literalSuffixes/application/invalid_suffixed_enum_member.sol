enum E {A, B, C}

function suffix(E) pure suffix returns (E) {}

contract C {
    E e = E.A suffix;
}
// ----
// ParserError 2314: (92-98): Expected ';' but got identifier
