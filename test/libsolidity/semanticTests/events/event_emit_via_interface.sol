interface I {
    event Event(address indexed _from, uint256 _value);
}

contract C {
    function emitEvent(uint256 _value) public {
        emit I.Event(msg.sender, _value);
    }
}

// ----
// emitEvent(uint256): 100 ->
// ~ emit Event(address,uint256): #0x1212121212121212121212121212120000000012, 0x64
