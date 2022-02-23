contract C {
    function f() public {
        (msg.sender).transfer(10);
    }
}
// ----
// TypeError 9862: (47-68): "send" and "transfer" are only available for objects of type "address payable", not "address".
