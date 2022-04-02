contract C {
    function f() public {
        address(this).send(10);
    }
}

// ----
// TypeError 9862: (47-65='address(this).send'): "send" and "transfer" are only available for objects of type "address payable", not "address".
