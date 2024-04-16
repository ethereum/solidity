contract C {
    function f() public view returns (uint ret) {
        assembly {
            let tload := sload(0)
            ret := tload
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// ParserError 5568: (98-103): Cannot use builtin function name "tload" as identifier name.
