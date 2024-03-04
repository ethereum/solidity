contract C {
    event E(uint256 value);
}

contract D {
    event E(uint256 value);

    function test() public {
        emit C.E(1);
        emit E(2);
    }
}

// ----
// test() ->
// ~ emit E(uint256): 0x01
// ~ emit E(uint256): 0x02
