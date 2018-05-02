contract C {
    // Fool parser into parsing a constructor as a function type.
    function f() {
      constructor() x;
    }
}
// ----
// ParserError: (118-118): Expected ';' but got identifier
