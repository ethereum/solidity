pragma experimental SMTChecker;

contract C
{
    uint x;
    address owner;

	modifier onlyOwner {
        if (msg.sender == owner) _;
    }

    function f() public onlyOwner {
    }

    function g(uint y) public {
        y = 1;
        if (y > x) f();
    }
}
