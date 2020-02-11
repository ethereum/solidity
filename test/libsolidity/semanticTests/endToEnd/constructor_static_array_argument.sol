contract C {
    uint public a;
    uint[3] public b;

    constructor(uint _a, uint[3] memory _b) public {
        a = _a;
        b = _b;
    }
}

// ----
// a() -> 1
// a():"" -> "1"
// b(uint256): 0) -> 2
// b(uint256):"0" -> "2"
// b(uint256): 1) -> 3
// b(uint256):"1" -> "3"
// b(uint256): 2) -> 4
// b(uint256):"2" -> "4"
