contract C {
    function g() public returns(uint) {
        return 7;
    }

    function f(function() external returns(uint) g) public returns(uint) {
        return g();
    }
}

// ----
// callContractFunction( "f(function): m_contractAddress.asBytes() + FixedHash<4>(util::keccak256("g())).asBytes() + bytes(32 - 4 - 20, 0)  -> 7
// f(function):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,e2,17,9b,8e,0,0,0,0,0,0,0,0]" -> "7"
