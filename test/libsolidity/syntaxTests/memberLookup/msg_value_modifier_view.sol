contract C {
    modifier costs(uint _amount) { require(msg.value >= _amount); _; }
    function f() costs(1 ether) public view {}
}
// ----
// TypeError: (101-115): This modifier uses "msg.value" and thus the function has to be payable or internal.
