contract C {
    function f() pure public {
        abi.decode("", (type(uint)));
    }
}
// ----
// TypeError 1039: (68-78): Argument has to be a type name.
