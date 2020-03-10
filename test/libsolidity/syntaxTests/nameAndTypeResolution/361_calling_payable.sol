contract receiver { function pay() payable public {} }
contract test {
    function f() public { (new receiver()).pay{value: 10}(); }
    function g() public { (new receiver()).pay.value(10)(); }
    receiver r = new receiver();
    function h() public { r.pay{value: 10}(); }
    function i() public { r.pay.value(10)(); }
}
// ----
// Warning: (160-186): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning: (303-314): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
