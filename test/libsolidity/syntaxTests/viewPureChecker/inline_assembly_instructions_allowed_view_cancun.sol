contract C {
    function f() public view {
        assembly {
            pop(blobhash(0))
            pop(blobbasefee())
            mcopy(1, 2, 3)
            pop(tload(0))
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
