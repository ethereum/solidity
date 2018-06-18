contract C {
    modifier A { _; }
    apply A {
        function f() {}
    }
}
// ----
// Warning: (57-72): No visibility specified. Defaulting to "public". 
// Warning: (57-72): Function state mutability can be restricted to pure
