contract C {
    uint public state = 0;
    constructor(uint _state) public payable {
        state = _state;
    }
    function balance() public payable returns (uint256) {
        return address(this).balance;
    }
    function update(uint _state) public {
        state = _state;
    }
}
// ----
// constructor(), 2 ether: 3 ->
// state() -> 3
// balance() -> 2
// update(uint256): 4
// state() -> 4
