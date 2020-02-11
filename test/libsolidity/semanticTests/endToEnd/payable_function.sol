contract C {
    uint public a;

    function f() payable public returns(uint) {
        return msg.value;
    }
    fallback() external payable {
        a = msg.value + 1;
    }
}

// ----
// callContractFunctionWithValue("f(): 27 -> 27
// f():"" -> "27"
// callContractFunctionWithValue(": 27 -> 
// :"" -> ""
// a() -> 28
// a():"" -> "28"
