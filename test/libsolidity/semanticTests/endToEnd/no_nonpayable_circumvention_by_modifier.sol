contract C {
    modifier tryCircumvent {
        if (false) _; // avoid the function, we should still not accept ether
    }

    function f() tryCircumvent public returns(uint) {
        return msgvalue();
    }

    function msgvalue() internal returns(uint) {
        return msg.value;
    }
}

// ====
// compileViaYul: also
// ----
// callContractFunctionWithValue("f(): 27 -> 
// f():"" -> ""
