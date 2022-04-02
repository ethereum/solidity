contract C {
    function f() pure { }
}
// ----
// SyntaxError 4937: (17-38='function f() pure { }'): No visibility specified. Did you intend to add "public"?
