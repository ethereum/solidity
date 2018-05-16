contract C {
    // Fool parser into parsing a constructor as a function type.
    function f() {
      constructor() x;
    }
}
// ----
// ParserError: (118-119): Expected ';' but got identifier
