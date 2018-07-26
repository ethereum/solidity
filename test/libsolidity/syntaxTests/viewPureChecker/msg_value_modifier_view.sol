contract C {
    modifier m(uint _amount, uint _avail) { require(_avail >= _amount); _; }
    function f() m(1 ether, msg.value) public view {}
}
// ----
// Warning: (118-127): "msg.value" used in non-payable function. Do you want to add the "payable" modifier to this function?
