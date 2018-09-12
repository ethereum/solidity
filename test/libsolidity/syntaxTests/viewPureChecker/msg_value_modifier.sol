contract C {
    modifier m(uint _amount, uint _avail) { require(_avail >= _amount); _; }
    function f() m(1 ether, msg.value) public pure {}
}
// ----
// TypeError: (118-127): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
