contract C {
    uint public a;

    function f() public returns(uint) {
        return msgvalue();
    }

    function msgvalue() internal returns(uint) {
        return msg.value;
    }
    fallback() external {
        update();
    }

    function update() internal {
        a = msg.value + 1;
    }

}

// ----
// callContractFunctionWithValue("f(): 27 -> 
// f():"" -> ""
// " -> 
// :"" -> ""
// a() -> 1
// a():"" -> "1"
// callContractFunctionWithValue(": 27 -> 
// :"" -> ""
// a() -> 1
// a():"" -> "1"
// callContractFunctionWithValue("a(): 27 -> 
// a():"" -> ""
