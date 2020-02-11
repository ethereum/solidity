// Tests that rational numbers (even negative ones) are encoded properly.
contract C {
    function f() public pure returns(bytes memory) {
        return abi.encode(1, -2);
    }
}

// ----
// f() ->  0x20, 0x40, 1, -2 
// f():"" -> "32, 64, 1, 115792089237316195423570985008687907853269984665640564039457584007913129639934"
