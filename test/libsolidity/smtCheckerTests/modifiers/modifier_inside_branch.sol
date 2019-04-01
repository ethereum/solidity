pragma experimental SMTChecker;

contract C {
    address owner;
    modifier onlyOwner {
        if (msg.sender == owner) _;
    }
    function g() public onlyOwner {
    }
    function f(uint x) public {
        if (x > 0) g();
    }
}
