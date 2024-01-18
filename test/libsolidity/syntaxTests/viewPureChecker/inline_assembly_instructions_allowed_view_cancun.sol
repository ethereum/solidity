contract C {
    function f() public view {
        assembly {
            pop(blobhash(0))
            pop(blobbasefee())
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
