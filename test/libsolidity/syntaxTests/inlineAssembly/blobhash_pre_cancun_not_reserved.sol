contract C {
    function f() public pure returns (uint ret) {
        assembly {
            let blobhash := 1
            ret := blobhash
        }
    }
    function g() public pure returns (uint ret) {
        assembly {
            function blobhash() -> r {
                r := 1000
            }
            ret := blobhash()
        }
    }
}
// ====
// EVMVersion: <=shanghai
// ----
