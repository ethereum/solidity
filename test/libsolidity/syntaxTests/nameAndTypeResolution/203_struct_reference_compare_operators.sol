contract test {
  struct s {uint a;}
  s x;
  s y;
  fallback() external {
    x == y;
  }
}
// ----
// TypeError 2271: (79-85): Operator == not compatible with types struct test.s storage ref and struct test.s storage ref. A user-defined operator not found.
