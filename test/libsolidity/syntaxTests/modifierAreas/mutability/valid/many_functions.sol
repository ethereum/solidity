contract C {
    apply pure {
        function f() {}
        function g() {}
    }
}
// ----
// Warning: (38-53): No visibility specified. Defaulting to "public". 
// Warning: (62-77): No visibility specified. Defaulting to "public". 
