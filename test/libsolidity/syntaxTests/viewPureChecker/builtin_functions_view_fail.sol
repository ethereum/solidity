contract C {
    function f() view public {
        address(this).transfer(1);
    }
    function g() view public {
        require(address(this).send(2));
    }
    function h() view public {
        selfdestruct(address(this));
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
// TypeError: (52-77): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (132-153): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (201-228): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (293-323): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (414-436): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
