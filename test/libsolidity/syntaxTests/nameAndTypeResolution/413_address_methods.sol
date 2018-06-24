contract C {
    function f() public {
        address addr;
        uint balance = addr.balance;
        bool callRet = addr.call();
        bool delegatecallRet = addr.delegatecall();
        bool sendRet = addr.send(1);
        addr.transfer(1);
        balance; callRet; delegatecallRet; sendRet;
    }
}
// ----
