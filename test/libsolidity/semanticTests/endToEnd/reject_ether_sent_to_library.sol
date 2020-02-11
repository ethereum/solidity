library lib {}
contract c {
    constructor() public payable {}

    function f(address payable x) public returns(bool) {
        return x.send(1);
    }
    receive() external payable {}
}

// ----

library lib {}
contract c {
    constructor() public payable {}

    function f(address payable x) public returns(bool) {
        return x.send(1);
    }
    receive() external payable {}
}

// ----
// f(address): encodeArgs(u160(libraryAddress)) -> false
// f(address):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "0"
// f(address): encodeArgs(u160(m_contractAddress)) -> true
// f(address):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1"
