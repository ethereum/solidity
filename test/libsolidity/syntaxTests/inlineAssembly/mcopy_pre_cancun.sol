contract C {
    function f() public pure {
        assembly {
            mcopy()
        }
    }
}
// ====
// EVMVersion: =shanghai
// ----
// TypeError 7755: (75-80): The "mcopy" instruction is only available for Cancun-compatible VMs (you are currently compiling for "shanghai").
