contract C {
    enum Color { red, green, blue }
    function f() pure public {
        abi.decode("", (Color));
    }
}
// ----
// Warning 6133: (88-111): Statement has no effect.
