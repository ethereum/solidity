contract C {
    function f() public pure {
        assembly { pop(blobbasefee()) }
    }
    function g() public pure returns (uint) {
        return block.blobbasefee;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 2527: (67-80): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (151-168): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
