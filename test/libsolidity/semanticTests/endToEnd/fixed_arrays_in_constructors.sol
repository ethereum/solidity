contract Creator {
    uint public r;
    address public ch;
    constructor(address[3] memory s, uint x) public {
        r = x;
        ch = s[2];
    }
}

// ----
r(): ""
ch(): ""
