contract C {
    event MyCustomEvent(uint);
    function f() public returns (uint8, uint8, int) {
        return ((MyCustomEvent, 8, MyCustomEvent));
    }
}

// ----
// TypeError 5992: (113-148): Return argument type tuple(event MyCustomEvent(uint256),int_const 8,event MyCustomEvent(uint256)) is not implicitly convertible to expected type tuple(uint8,uint8,int256).
