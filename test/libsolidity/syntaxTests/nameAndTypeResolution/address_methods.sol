contract C {
    function f() public {
        address addr;
        uint balance = addr.balance;
        bool callRet = addr.call();
        bool callcodeRet = addr.callcode();
        bool delegatecallRet = addr.delegatecall();
        bool sendRet = addr.send(1);
        addr.transfer(1);
        callRet; callcodeRet; delegatecallRet; sendRet;
    }
}
// ----
// Warning: (161-174): "callcode" has been deprecated in favour of "delegatecall".
// Warning: (69-81): Unused local variable.
