contract test {
    function f(bytes4 memory) public;
}
// ----
// TypeError: (31-37): Data location can only be given for array or struct types.
