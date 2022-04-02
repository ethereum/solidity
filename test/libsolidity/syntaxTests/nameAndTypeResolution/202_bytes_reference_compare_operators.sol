contract test { bytes a; bytes b; fallback() external { a == b; } }
// ----
// TypeError 2271: (56-62='a == b'): Operator == not compatible with types bytes storage ref and bytes storage ref
