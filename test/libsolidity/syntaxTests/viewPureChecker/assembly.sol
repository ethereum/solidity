contract C {
    struct S { uint x; }
    S s;
    function e() pure public {
        assembly { mstore(keccak256(0, 20), mul(s_slot, 2)) }
    }
    function f() pure public {
        uint x;
        assembly { x := 7 }
    }
    function g() view public {
        assembly { for {} 1 { pop(sload(0)) } { } pop(gas) }
    }
    function h() view public {
        assembly { function g() { pop(blockhash(20)) } }
    }
    function j() public {
        assembly { pop(call(0, 1, 2, 3, 4, 5, 6)) }
    }
    function k() public {
        assembly { pop(call(gas, 1, 2, 3, 4, 5, 6)) }
    }
}
