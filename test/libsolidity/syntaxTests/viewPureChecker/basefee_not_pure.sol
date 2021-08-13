contract C {
    function f() public pure {
        assembly { pop(basefee()) }
    }
    function g() public pure returns (uint) {
        return block.basefee;
    }
}
// ====
// EVMVersion: >=london
// ----
// TypeError 2527: (67-76): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (147-160): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
