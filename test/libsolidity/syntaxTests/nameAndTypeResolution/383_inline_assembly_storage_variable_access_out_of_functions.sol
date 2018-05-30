pragma experimental "v0.5.0";
contract test {
    uint a;
    function f() pure public {
        assembly {
            function g() -> x { x := a_slot }
        }
    }
}
