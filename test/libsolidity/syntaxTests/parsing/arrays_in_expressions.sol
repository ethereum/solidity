contract c {
    function f() public { c[10] a = 7; uint8[10 * 2] x; }
}
// ----
// Warning: (39-46): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// Warning: (52-67): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// TypeError: (39-50): Type int_const 7 is not implicitly convertible to expected type contract c[10] storage pointer.
// Warning: (52-67): Uninitialized storage pointer. Did you mean '<type> memory x'?
