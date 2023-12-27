contract C {
    function f() public pure returns (bytes32 ret){
        assembly {
            ret := blobhash(0)
        }
    }
    function g() public pure returns (bytes32) {
        return blobhash(0);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 2527: (103-114): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (195-206): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
