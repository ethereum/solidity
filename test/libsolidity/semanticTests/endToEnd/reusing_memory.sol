contract Helper {
    uint public flag;
    constructor(uint x) public {
        flag = x;
    }
}
contract Main {
    mapping(uint => uint) map;

    function f(uint x) public returns(uint) {
        map[x] = x;
        return (new Helper(uint(keccak256(abi.encodePacked(this.g(map[x])))))).flag();
    }

    function g(uint a) public returns(uint) {
        return map[a];
    }
}

// ----
// f(uint256): 52 -> 31997345449574252472561286867836691613551392380036115619611668045310140188353
