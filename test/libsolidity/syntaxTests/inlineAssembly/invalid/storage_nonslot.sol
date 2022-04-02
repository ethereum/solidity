contract test {
    uint x = 1;
    function f() public {
        assembly {
            let t := x.length
            x.length := 2
        }
    }
}
// ----
// TypeError 4656: (98-106='x.length'): State variables only support ".slot" and ".offset".
// TypeError 4656: (119-127='x.length'): State variables only support ".slot" and ".offset".
