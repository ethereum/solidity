contract C {
  function f() public {
    f[];
  }
}
// ----
// TypeError: (41-42): Indexed expression has to be a type, mapping or array (is function ())
