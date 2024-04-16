contract C {
    function f() public {
        assembly {
            tstore(0, 13)
            tload(0)
        }
    }
}
// ====
// EVMVersion: =shanghai
// ----
// TypeError 6243: (70-76): The "tstore" instruction is only available for Cancun-compatible VMs (you are currently compiling for "shanghai").
// TypeError 6243: (96-101): The "tload" instruction is only available for Cancun-compatible VMs (you are currently compiling for "shanghai").
