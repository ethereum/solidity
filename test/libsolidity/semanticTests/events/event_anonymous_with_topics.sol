contract ClientReceipt {
    event Deposit(address indexed _from, bytes32 indexed _id, uint indexed _value, uint indexed _value2, bytes32 data) anonymous;
    function deposit(bytes32 _id) public payable {
        emit Deposit(msg.sender, _id, msg.value, 2, "abc");
    }
}
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit <anonymous>: #0x1212121212121212121212121212120000000012, #0x1234, #0x12, #0x02, "abc"
