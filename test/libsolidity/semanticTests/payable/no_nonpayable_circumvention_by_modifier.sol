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
}
// ====
// compileToEwasm: also
// ----
// f(), 27 wei -> FAILURE
// balance -> 0
