struct S { uint x; }
S constant s;
// ----
// TypeError 9259: (21-33): Constants of non-value type not yet implemented.
// TypeError 4266: (21-33): Uninitialized "constant" variable.
