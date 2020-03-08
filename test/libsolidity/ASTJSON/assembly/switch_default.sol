contract C {
    function g() view public {
        assembly { switch 0 default {} }
    }
}

// ----
