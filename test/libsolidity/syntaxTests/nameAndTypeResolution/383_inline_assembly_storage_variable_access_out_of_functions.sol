contract test {
    uint a;
    function f() pure public {
        assembly {
            function g() -> x { x := a_slot }
        }
    }
}
