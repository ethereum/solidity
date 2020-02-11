library L {
    function f() public returns(uint) {
        return 7;
    }
}
contract C {
    function f() public payable returns(uint) {
        return L.f();
    }
}

// ----

library L {
    function f() public returns(uint) {
        return 7;
    }
}
contract C {
    function f() public payable returns(uint) {
        return L.f();
    }
}

// ----
// callContractFunctionWithValue("f(): 27 -> 7
// f():"" -> "7"
