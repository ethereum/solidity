contract C {
    function f() public pure {
        assembly {
            mcopy()
        }
    }
}
// ====
// EVMVersion: <cancun
// ----
// DeclarationError 4619: (75-80): Function "mcopy" not found.
