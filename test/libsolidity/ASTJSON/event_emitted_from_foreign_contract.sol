contract C {
    event E(address indexed sender);
}
contract D {
    function test(address sender) public {
        emit C.E(msg.sender);
    }
}

// ----
