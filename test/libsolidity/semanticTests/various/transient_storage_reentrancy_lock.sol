contract C {
    bool transient locked;
    modifier nonReentrant {
        require(!locked, "Reentrancy attempt");
        locked = true;
        _;
        locked = false;
    }

    function test(address newAddress, bool reentrancy) nonReentrant public {
        if (reentrancy)
            reentrantCall(newAddress);
    }

    function reentrantCall(address a) public {
        this.test(a, false);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// test(address,bool): 0x1234abcd, true -> FAILURE, hex"08c379a0", 0x20, 0x12, "Reentrancy attempt"
// test(address,bool): 0x1234abcd, false ->
