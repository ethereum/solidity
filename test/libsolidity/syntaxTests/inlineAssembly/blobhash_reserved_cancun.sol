contract C {
    function f() public pure returns (uint ret) {
        assembly {
            function blobhash() -> r {
                r := 1000
            }
            ret := blobhash()
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// ParserError 5568: (103-111): Cannot use builtin function name "blobhash" as identifier name.
