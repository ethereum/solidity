library Lib {
    function m() public returns(address) {
        return msg.sender;
    }
}
contract Test {
    address public sender;

    function f() public {
        sender = Lib.m();
    }
}

// ----

library Lib {
    function m() public returns(address) {
        return msg.sender;
    }
}
contract Test {
    address public sender;

    function f() public {
        sender = Lib.m();
    }
}

// ----
// f() -> 
// f():"" -> ""
// sender() -> u160(m_sender
// sender():"" -> "103164821458651970696730694074090566015747358738"
