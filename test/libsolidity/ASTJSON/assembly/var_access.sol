contract C {
    function f() pure public {
        uint x;
        assembly { x := 7 }
    }
}

// ----
