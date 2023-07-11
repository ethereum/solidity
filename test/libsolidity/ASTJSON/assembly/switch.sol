contract C {
    function f() pure public {
        assembly {
            let v := 0
            switch calldatasize()
            case 0 { v := 1 }
            default { v := 2 }
        }
    }
}
// ----
