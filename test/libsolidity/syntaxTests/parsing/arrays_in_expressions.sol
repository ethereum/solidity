contract c {
    function f() { c[10] a = 7; uint8[10 * 2] x; }
}
// ----
// Warning: (32-39): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// Warning: (45-60): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// TypeError: (32-43): Type int_const 7 is not implicitly convertible to expected type contract c[10] storage pointer.
// Warning: (45-60): Uninitialized storage pointer. Did you mean '<type> memory x'?
