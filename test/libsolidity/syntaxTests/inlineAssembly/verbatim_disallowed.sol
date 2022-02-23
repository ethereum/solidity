contract C {
    function f() pure public {
        assembly {
            let x := verbatim_1o(hex"6001")
        }
    }
}
// ====
// optimize-yul: true
// ----
// DeclarationError 4619: (84-95): Function "verbatim_1o" not found.
// DeclarationError 3812: (75-106): Variable count mismatch for declaration of "x": 1 variables and 0 values.
