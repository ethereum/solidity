contract C {
    mapping(address => address payable) public m;
    function f(address payable arg) public returns (address payable r) {
        address payable a = m[arg];
        r = arg;
        address c = address(this);
        m[c] = address(0);
    }
}

// ----
