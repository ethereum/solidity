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
// EVMVersion: >=byzantium
// ----
