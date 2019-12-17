contract C {
    function f() pure public {
        assembly {
            let f := 0
            switch calldatasize()
            case 0 { f := 1 }
            default { f := 2 }
        }
    }
}

// ----
