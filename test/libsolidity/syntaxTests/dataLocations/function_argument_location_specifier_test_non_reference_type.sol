contract test {
    function f(bytes4 memory) public;
}
// ----
// TypeError 6651: (31-44): Data location can only be specified for array, struct or mapping types, but "memory" was given.
