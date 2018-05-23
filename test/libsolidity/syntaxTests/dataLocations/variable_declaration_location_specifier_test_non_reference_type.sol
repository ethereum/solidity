contract test {
    function f() {
      uint storage a1;
      bytes16 storage b1;
      uint memory a2;
      bytes16 memory b2;
    }
}
// ----
// TypeError: (41-56): Data location can only be given for array or struct types.
// TypeError: (64-82): Data location can only be given for array or struct types.
// TypeError: (90-104): Data location can only be given for array or struct types.
// TypeError: (112-129): Data location can only be given for array or struct types.
