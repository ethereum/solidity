contract C {
    struct Mail {
        address from;
        address to;
        string contents;
    }

    function f() public pure returns(bool) {
        return type(Mail).typehash == keccak256("Mail(address from,address to,string contents)");
    }
}
// ----
// f() -> true
