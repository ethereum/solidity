contract C {
    apply internal {
        function f() {}
        function g() {}
    }
}
// ----
// Warning: (42-57): Function state mutability can be restricted to pure
// Warning: (66-81): Function state mutability can be restricted to pure
