contract C {
    modifier costs(uint _amount) { require(msg.value >= _amount); _; }
    function f() costs(1 ether) public pure {}
}
// ----
// TypeError: (101-115): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
