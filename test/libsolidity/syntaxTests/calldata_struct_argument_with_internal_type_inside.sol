contract C {
  struct S {
    function() a;
  }
  function f(S[2] calldata) {}
}
// ----
// SyntaxError 4937: (50-78): No visibility specified. Did you intend to add "public"?
// TypeError 4103: (61-74): Internal type is not allowed for public or external functions.