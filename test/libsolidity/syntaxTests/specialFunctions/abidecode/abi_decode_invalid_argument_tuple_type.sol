contract C {
    function f() pure public {
        abi.decode("", ((uint, int)));
    }
}
// ----
// TypeError 1039: (68-79): Argument has to be a type name.
