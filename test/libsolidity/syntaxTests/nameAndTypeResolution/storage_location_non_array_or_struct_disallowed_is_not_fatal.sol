contract C {
    function f(uint storage a) public {
        a = f;
    }
}
// ----
// TypeError: (28-42): Data location can only be given for array or struct types.
