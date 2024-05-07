contract C {
    function m() public {
        assembly { let x := "abc" }
    }
}

// ----
