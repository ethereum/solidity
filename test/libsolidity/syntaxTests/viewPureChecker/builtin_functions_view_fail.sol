contract C {
    function f() view public {
        payable(this).transfer(1);
    }
    function g() view public {
        require(payable(this).send(2));
    }
    function h() view public {
        selfdestruct(payable(this));
    }
    function i() view public {
        (bool success,) = address(this).delegatecall("");
        require(success);
    }
    function j() view public {
        (bool success,) = address(this).call("");
        require(success);
    }
    receive() payable external {
    }
}
// ----
// Warning 5159: (201-213): "selfdestruct" has been deprecated. Note that, starting from the Cancun hard fork, the underlying opcode no longer deletes the code and data associated with an account and only transfers its Ether to the beneficiary, unless executed in the same transaction in which the contract was created (see EIP-6780). Any use in newly deployed contracts is strongly discouraged even if the new behavior is taken into account. Future changes to the EVM might further reduce the functionality of the opcode.
// TypeError 8961: (52-77): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (132-153): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (201-228): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (293-323): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (414-436): Function cannot be declared as view because this expression (potentially) modifies the state.
