contract test {
  struct s {uint a;}
  s x;
  s y;
  fallback() external {
    x == y;
  }
}
// ----
// TypeError 2271: (79-85): Binary operator == not compatible with types struct test.s storage ref and struct test.s storage ref. No matching user-defined operator found.
