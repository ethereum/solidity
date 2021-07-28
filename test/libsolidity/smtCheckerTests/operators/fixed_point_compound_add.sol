contract C {
  fixed[] b;
  function f() internal { b[0] += 1; }
}
// ====
// SMTEngine: all
// ----
// TypeError 7366: (52-61): Operator += not compatible with types fixed128x18 and int_const 1
