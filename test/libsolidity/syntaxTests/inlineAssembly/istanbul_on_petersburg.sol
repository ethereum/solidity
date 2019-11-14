contract C {
    function f() pure external returns (uint id) {
        assembly {
            id := chainid()
        }
    }
    function g() view external returns (uint sb) {
        assembly {
            sb := selfbalance()
        }
    }
}
// ====
// EVMVersion: =petersburg
// ----
// TypeError: (101-110): The "chainid" instruction is only available for Istanbul-compatible VMs  (you are currently compiling for "petersburg").
// TypeError: (215-228): The "selfbalance" instruction is only available for Istanbul-compatible VMs  (you are currently compiling for "petersburg").
