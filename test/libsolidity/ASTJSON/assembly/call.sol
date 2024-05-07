contract C {
    function j() public {
        assembly { pop(call(0, 1, 2, 3, 4, 5, 6)) }
    }
}

// ----
