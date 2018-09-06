contract C {
    modifier costs(uint _amount) { require(msg.value >= _amount); _; }
    function f() costs(1 ether) public payable {}
}
