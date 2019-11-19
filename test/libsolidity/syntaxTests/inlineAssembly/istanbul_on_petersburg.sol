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
// DeclarationError: (101-108): Function not found.
// DeclarationError: (95-110): Variable count does not match number of values (1 vs. 0)
// DeclarationError: (215-226): Function not found.
// DeclarationError: (209-228): Variable count does not match number of values (1 vs. 0)
