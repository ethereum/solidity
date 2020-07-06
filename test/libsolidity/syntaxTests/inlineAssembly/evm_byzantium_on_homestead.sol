contract C {
    function f() pure external {
        assembly {
            let s := returndatasize()
            returndatacopy(0, 0, s)
        }
    }
    function g() view external returns (uint ret) {
        assembly {
            ret := staticcall(0, gas(), 0, 0, 0, 0)
        }
    }
}
// ====
// EVMVersion: =homestead
// ----
// TypeError 7756: (86-100): The "returndatasize" instruction is only available for Byzantium-compatible VMs (you are currently compiling for "homestead").
// DeclarationError 3812: (77-102): Variable count mismatch: 1 variables and 0 values.
// TypeError 7756: (115-129): The "returndatacopy" instruction is only available for Byzantium-compatible VMs (you are currently compiling for "homestead").
// TypeError 1503: (245-255): The "staticcall" instruction is only available for Byzantium-compatible VMs (you are currently compiling for "homestead").
// DeclarationError 8678: (238-277): Variable count does not match number of values (1 vs. 0)
