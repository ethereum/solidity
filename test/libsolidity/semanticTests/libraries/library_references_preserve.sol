library L1 {
    function add(uint256 a, uint256 b) external pure returns (uint256) {
        return a + b + 1;
    }
}

library L2 {
    function add(uint256 a, uint256 b) external pure returns (uint256) {
        return a + b + 2;
    }
}

contract A {
    uint256 sum;
    constructor() {
        sum = L1.add(1, 2);
    }
    function getSum() external view returns(uint256) {
        return sum;
    }
}

contract B {
    uint256 sum;
    constructor() {
        sum = L2.add(1, 2);
    }
    function getSum() external view returns(uint256) {
        return sum;
    }
}

contract C {
    A a = new A();
    B b = new B();
    function aSum() external view returns(uint256) {
        return a.getSum();
    }
    function bSum() external view returns(uint256) {
        return b.getSum();
    }
}

// ----
// library: L1
// library: L2
// aSum() -> 4
// bSum() -> 5
