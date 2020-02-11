contract Test {
    function() internal x;

    function f() public returns(uint r) {
        function() internal t = x;
        assembly {
            r: = t
        }
    }
}

// ----
f(): ""
