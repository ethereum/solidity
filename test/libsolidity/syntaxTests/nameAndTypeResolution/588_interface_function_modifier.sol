interface I {
  function f() external m pure returns (uint);
  modifier m() { _; }
}
// ----
// SyntaxError: (16-60): Functions in interfaces cannot have modifiers.
