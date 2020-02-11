library Lib {
    struct S {
        uint x;
    }
    // a direct call to this should revert
    function np(S storage s) public returns(address) {
        s.x = 3;
        return msg.sender;
    }
    // a direct call to this is fine
    function v(S storage) public view returns(address) {
        return msg.sender;
    }
    // a direct call to this is fine
    function pu() public pure returns(uint) {
        return 2;
    }
}
contract Test {
    Lib.S public s;

    function np() public returns(address) {
        return Lib.np(s);
    }

    function v() public view returns(address) {
        return Lib.v(s);
    }

    function pu() public pure returns(uint) {
        return Lib.pu();
    }
}

// ----
// np(Lib.S storage): 0 -> 
// np(Lib.S storage):"0" -> ""
// v(Lib.S storage): 0 -> u160(m_sender
// v(Lib.S storage):"0" -> "103164821458651970696730694074090566015747358738"
// pu() -> 2
// pu():"" -> "2"

library Lib {
    struct S {
        uint x;
    }
    // a direct call to this should revert
    function np(S storage s) public returns(address) {
        s.x = 3;
        return msg.sender;
    }
    // a direct call to this is fine
    function v(S storage) public view returns(address) {
        return msg.sender;
    }
    // a direct call to this is fine
    function pu() public pure returns(uint) {
        return 2;
    }
}
contract Test {
    Lib.S public s;

    function np() public returns(address) {
        return Lib.np(s);
    }

    function v() public view returns(address) {
        return Lib.v(s);
    }

    function pu() public pure returns(uint) {
        return Lib.pu();
    }
}

// ----
// s() -> 0
// s():"" -> "0"
// np() -> u160(m_sender
// np():"" -> "103164821458651970696730694074090566015747358738"
// s() -> 3
// s():"" -> "3"
// v() -> u160(m_sender
// v():"" -> "103164821458651970696730694074090566015747358738"
// pu() -> 2
// pu():"" -> "2"
