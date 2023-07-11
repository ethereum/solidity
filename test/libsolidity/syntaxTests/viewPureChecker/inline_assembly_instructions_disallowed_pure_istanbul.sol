contract C {
    function f() public pure {
        assembly {
            pop(selfbalance())
            pop(chainid())
        }
    }
}
// ====
// EVMVersion: >=istanbul
// ----
// TypeError 2527: (79-92): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (110-119): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
