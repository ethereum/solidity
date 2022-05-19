contract ClientReceipt {
    bytes x;
    event Deposit(uint fixeda, bytes dynx, uint fixedb);
    function deposit() public {
        x.push("A");
        x.push("B");
        x.push("C");
        emit Deposit(10, x, 15);
    }
}
// ----
// deposit() ->
// ~ emit Deposit(uint256,bytes,uint256): 0x0a, 0x60, 0x0f, 0x03, "ABC"
