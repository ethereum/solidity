contract test { bytes a; bytes b; fallback() external { a == b; } }
// ----
// TypeError 2271: (56-62): Built-in binary operator == cannot be applied to types bytes storage ref and bytes storage ref.
