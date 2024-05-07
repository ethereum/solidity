contract C {
    function g() view public {
        assembly { switch 0 case 0 {} default {} }
    }
}

// ----
