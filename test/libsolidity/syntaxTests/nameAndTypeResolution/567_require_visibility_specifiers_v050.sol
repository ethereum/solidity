pragma experimental "v0.5.0";
contract C {
    function f() pure { }
}
// ----
// SyntaxError: (47-68): No visibility specified. Did you intend to add "public"?
