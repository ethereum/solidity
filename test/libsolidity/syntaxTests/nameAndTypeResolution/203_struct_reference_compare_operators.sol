contract test {
  struct s {uint a;}
  s x;
  s y;
  function() external {
    x == y;
  }
}
// ----
// TypeError: (79-85): Operator == not compatible with types struct test.s storage ref and struct test.s storage ref
