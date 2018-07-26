contract C {
    modifier costs(uint _amount) { require(msg.value >= _amount); _; }
    function f() costs(1 ether) public view {}
}
// ----
// Warning: (101-115): This modifier uses "msg.value" and thus the function should be payable.
