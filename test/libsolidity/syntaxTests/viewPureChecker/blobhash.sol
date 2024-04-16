contract C {
    function f() public view returns (bytes32 ret){
        assembly {
            ret := blobhash(0)
        }
    }
    function g() public view returns (bytes32) {
        return blobhash(0);
    }
}
// ====
// EVMVersion: >=cancun
// ----
