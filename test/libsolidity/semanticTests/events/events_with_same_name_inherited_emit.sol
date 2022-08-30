contract A {
    event Deposit();
}

contract B {
    event Deposit(address _addr);
}

contract ClientReceipt is A, B {
    event Deposit(address _addr, uint _amount);
    function deposit() public returns (uint) {
        emit Deposit();
        return 1;
    }
    function deposit(address _addr) public returns (uint) {
        emit Deposit(_addr);
        return 1;
    }
    function deposit(address _addr, uint _amount) public returns (uint) {
        emit Deposit(_addr, _amount);
        return 1;
    }
}
// ----
// deposit() -> 1
// ~ emit Deposit()
// deposit(address): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988 -> 1
// ~ emit Deposit(address): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988
// deposit(address,uint256): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988, 100 -> 1
// ~ emit Deposit(address,uint256): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988, 0x64
