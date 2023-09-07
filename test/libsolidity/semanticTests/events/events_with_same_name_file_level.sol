event Deposit();
event Deposit(address _addr);
event Deposit(address _addr, uint _amount);
event Deposit(address _addr, bool _flag);

contract ClientReceipt {
    function deposit() public returns (uint) {
        emit Deposit();
        return 1;
    }
    function deposit(address _addr) public returns (uint) {
        emit Deposit(_addr);
        return 2;
    }
    function deposit(address _addr, uint _amount) public returns (uint) {
        emit Deposit(_addr, _amount);
        return 3;
    }
    function deposit(address _addr, bool _flag) public returns (uint) {
        emit Deposit(_addr, _flag);
        return 4;
    }
}
// ----
// deposit() -> 1
// ~ emit Deposit()
// deposit(address): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988 -> 2
// ~ emit Deposit(address): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988
// deposit(address,uint256): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988, 100 -> 3
// ~ emit Deposit(address,uint256): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988, 0x64
// deposit(address,bool): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988, false -> 4
// ~ emit Deposit(address,bool): 0x5082a85c489be6aa0f2e6693bf09cc1bbd35e988, false
