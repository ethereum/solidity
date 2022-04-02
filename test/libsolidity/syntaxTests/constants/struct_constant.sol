struct S { uint x; }
S constant s;
// ----
// TypeError 9259: (21-33='S constant s'): Only constants of value type and byte array type are implemented.
