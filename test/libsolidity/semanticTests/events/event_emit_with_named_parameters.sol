contract C {
    event E(uint256 a, uint256 b, string c);

    function trigger() public {
        emit E(2, 7, "event");
        emit E({b: 7, a: 2, c: "event"});
        emit E({a: 2, c: "event", b: 7});
        emit E({c: "event", a: 2, b: 7});
        emit E({a: 2, b: 7, c: "event"});
    }
}
// ----
// trigger() ->
// ~ emit E(uint256,uint256,string): 0x02, 0x07, 0x60, 0x05, "event"
// ~ emit E(uint256,uint256,string): 0x02, 0x07, 0x60, 0x05, "event"
// ~ emit E(uint256,uint256,string): 0x02, 0x07, 0x60, 0x05, "event"
// ~ emit E(uint256,uint256,string): 0x02, 0x07, 0x60, 0x05, "event"
// ~ emit E(uint256,uint256,string): 0x02, 0x07, 0x60, 0x05, "event"
