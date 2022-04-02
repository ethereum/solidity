contract C {
    function f(uint storage a) public { }
}
// ----
// TypeError 6651: (28-42='uint storage a'): Data location can only be specified for array, struct or mapping types, but "storage" was given.
