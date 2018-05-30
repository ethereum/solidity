contract test { bytes a; bytes b; function() public { a == b; } }
// ----
// TypeError: (54-60): Operator == not compatible with types bytes storage ref and bytes storage ref
