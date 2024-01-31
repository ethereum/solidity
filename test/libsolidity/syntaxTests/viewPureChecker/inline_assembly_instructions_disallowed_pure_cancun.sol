contract C {
    function f() public pure {
        assembly {
            pop(blobhash(0))
            pop(blobbasefee())
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 2527: (79-90): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (108-121): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
