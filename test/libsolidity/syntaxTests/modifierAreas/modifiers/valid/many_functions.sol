contract C {
    modifier A { _; }
    apply A {
        function f() {}
        function g() {}
    }
}
// ----
// Warning: (57-72): No visibility specified. Defaulting to "public". 
// Warning: (81-96): No visibility specified. Defaulting to "public". 
// Warning: (57-72): Function state mutability can be restricted to pure
// Warning: (81-96): Function state mutability can be restricted to pure
