contract C {
    function f(uint storage a) public { }
}
// ----
// TypeError: (28-42): Data location can only be specified for array, struct or mapping types, but "storage" was given.
