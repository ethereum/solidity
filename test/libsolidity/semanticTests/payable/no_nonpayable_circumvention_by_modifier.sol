contract C {
    modifier tryCircumvent {
        if (false) _; // avoid the function, we should still not accept ether
    }
    function f() tryCircumvent public returns (uint) {
        return msgvalue();
    }
    function msgvalue() internal returns (uint) {
        return msg.value;
    }
    // TODO: remove this helper function once isoltest supports balance checking
    function balance() external returns (uint) {
        return address(this).balance;
    }
}
// ----
// f(), 27 wei -> FAILURE
// balance() -> 0
