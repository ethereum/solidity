contract test { struct s {uint a;} s x; s y; function() public { x == y; } }
// ----
// TypeError: (65-71): Operator == not compatible with types struct test.s storage ref and struct test.s storage ref
