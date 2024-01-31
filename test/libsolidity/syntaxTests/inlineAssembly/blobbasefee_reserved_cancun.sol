contract C {
    function f() public view returns (uint ret) {
        assembly {
            let blobbasefee := sload(0)
            ret := blobbasefee
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// ParserError 5568: (98-109): Cannot use builtin function name "blobbasefee" as identifier name.
