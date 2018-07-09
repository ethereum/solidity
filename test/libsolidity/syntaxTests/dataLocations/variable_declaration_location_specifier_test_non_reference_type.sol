contract test {
    function f() public {
      uint storage a1;
      bytes16 storage b1;
      uint memory a2;
      bytes16 memory b2;
    }
}
// ----
// TypeError: (48-63): Data location can only be given for array or struct types.
// TypeError: (71-89): Data location can only be given for array or struct types.
// TypeError: (97-111): Data location can only be given for array or struct types.
// TypeError: (119-136): Data location can only be given for array or struct types.
