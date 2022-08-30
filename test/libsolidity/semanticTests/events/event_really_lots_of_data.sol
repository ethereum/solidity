contract ClientReceipt {
    event Deposit(uint fixeda, bytes dynx, uint fixedb);
    function deposit() public {
        emit Deposit(10, msg.data, 15);
    }
}
// ----
// deposit() ->
// ~ emit Deposit(uint256,bytes,uint256): 0x0a, 0x60, 0x0f, 0x04, 0xd0e30db000000000000000000000000000000000000000000000000000000000
