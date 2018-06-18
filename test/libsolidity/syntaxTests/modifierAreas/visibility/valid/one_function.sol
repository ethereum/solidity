contract C {
    apply private {
        function f() {}
    }
}
// ----
// Warning: (41-56): Function state mutability can be restricted to pure
