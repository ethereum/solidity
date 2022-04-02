contract test {
    function f() public {
      uint storage a1;
      bytes16 storage b1;
      uint memory a2;
      bytes16 memory b2;
    }
}
// ----
// TypeError 6651: (48-63='uint storage a1'): Data location can only be specified for array, struct or mapping types, but "storage" was given.
// TypeError 6651: (71-89='bytes16 storage b1'): Data location can only be specified for array, struct or mapping types, but "storage" was given.
// TypeError 6651: (97-111='uint memory a2'): Data location can only be specified for array, struct or mapping types, but "memory" was given.
// TypeError 6651: (119-136='bytes16 memory b2'): Data location can only be specified for array, struct or mapping types, but "memory" was given.
