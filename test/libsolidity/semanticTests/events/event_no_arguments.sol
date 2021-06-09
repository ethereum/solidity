contract ClientReceipt {
    event Deposit();
    function deposit() public {
        emit Deposit();
    }
}
// ====
// compileViaYul: also
// ----
// deposit() ->
// ~ emit Deposit()
