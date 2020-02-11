contract D {
    constructor() public payable {}
}
contract C {
    uint public x;
    constructor() public payable {}

    function f(uint amount) public returns(D) {
        x++;
        return (new D).value(amount)();
    }

    function stack(uint depth) public returns(address) {
        if (depth < 1024)
            return this.stack(depth - 1);
        else
            return address(f(0));
    }
}

// ----
f(uint256): "20" // x() -> 1
// x():"" -> "1"
// f(uint256): 20 -> 
// f(uint256):"20" -> ""
// x() -> 1
// x():"" -> "1"
// stack(uint256): 1023 -> 
// stack(uint256):"1023" -> ""
// x() -> 1
// x():"" -> "1"
