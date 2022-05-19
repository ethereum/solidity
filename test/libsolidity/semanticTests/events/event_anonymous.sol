contract ClientReceipt {
    event Deposit() anonymous;
    function deposit() public {
        emit Deposit();
    }
}
// ----
// deposit() ->
// ~ emit <anonymous>
