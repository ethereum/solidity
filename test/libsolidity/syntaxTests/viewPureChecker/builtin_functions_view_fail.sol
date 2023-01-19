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
// Warning 5159: (201-213): "selfdestruct" has been deprecated. The underlying opcode will eventually undergo breaking changes, and its use is not recommended.
// TypeError 8961: (52-77): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (132-153): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (201-228): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (293-323): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (414-436): Function cannot be declared as view because this expression (potentially) modifies the state.
