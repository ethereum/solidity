contract C {
    function f() public {
        assembly {
            tstore(0, 13)
            tload(0)
        }
    }
}
// ====
// EVMVersion: <cancun
// ----
// DeclarationError 4619: (70-76): Function "tstore" not found.
// DeclarationError 4619: (96-101): Function "tload" not found.
