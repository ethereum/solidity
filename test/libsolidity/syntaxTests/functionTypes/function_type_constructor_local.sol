contract C {
    // Fool parser into parsing a constructor as a function type.
    function f() {
      constructor() x;
    }
}
// ----
// ParserError: (104-115): Expected primary expression.
