contract C {
    function f() public view {
        assembly {
            pop(blobhash(0))
            pop(blobbasefee())
            mcopy(1, 2, 3)
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
