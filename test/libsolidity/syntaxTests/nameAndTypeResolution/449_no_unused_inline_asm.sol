contract C {
    function f() pure public {
        uint a;
        assembly {
            a := 1
        }
    }
}
