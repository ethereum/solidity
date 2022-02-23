contract C {
    function f() public {
        (msg.sender).send(10);
    }
}
// ----
// TypeError 9862: (47-64): "send" and "transfer" are only available for objects of type "address payable", not "address".
