contract C {
    function f() pure external {
        assembly {
            pop(chainid())
        }
    }
    function g() view external {
        assembly {
            pop(selfbalance())
        }
    }
}
// ====
// EVMVersion: >=istanbul
// ----
