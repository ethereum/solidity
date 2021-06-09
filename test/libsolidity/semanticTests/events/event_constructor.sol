contract ClientReceipt {
    event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
    constructor() {
        emit Deposit(msg.sender, bytes32("abc"), 7);
    }
}
// ====
// compileViaYul: also
// ----
// constructor()
// ~ emit Deposit(address,bytes32,uint256): #0x1212121212121212121212121212120000000012, #"abc", 0x07
