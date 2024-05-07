contract C {
    function g() view public {
        assembly { for {} 1 { pop(sload(0)) } { break continue } }
    }
}

// ----
