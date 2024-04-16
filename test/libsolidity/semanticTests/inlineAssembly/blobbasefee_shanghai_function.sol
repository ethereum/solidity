contract C {
    function f() public view returns (uint ret) {
        assembly {
            let blobbasefee := 999
            ret := blobbasefee
        }
    }
    function g() public pure returns (uint ret) {
        assembly {
            function blobbasefee() -> r {
                r := 1000
            }
            ret := blobbasefee()
        }
    }
}
// ====
// EVMVersion: <=shanghai
// ----
// f() -> 999
// g() -> 1000
