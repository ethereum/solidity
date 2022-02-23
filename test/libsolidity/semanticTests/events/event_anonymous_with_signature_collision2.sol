contract ClientReceipt {
    event Withdraw(uint _value, string owner);
    event Deposit(uint256 indexed _from, bytes32 indexed _id, uint _value) anonymous;
    function deposit(bytes32 _id) public payable {
        emit Deposit(0x5ddaa77ac5bda319ba947e31bee594711f39ed1b20d079d438dbad5ed729fb30, _id, msg.value); // 0x5ddaa77a -> 'Withdraw(uint256,string)'
    }
}
// ====
// compileViaYul: also
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit Withdraw(uint256,string): #0x1234, 0x12
