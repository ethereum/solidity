contract receiver { function pay() payable public {} }
contract test {
    function f() public { (new receiver()).pay.value(10)(); }
    receiver r = new receiver();
    function g() public { r.pay.value(10)(); }
}
