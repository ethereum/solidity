contract C {
    function f() public {
        address(this).send(10);
    }
}

// ----
// TypeError: (47-65): "send" and "transfer" are only available for objects of type "address payable", not "address".
