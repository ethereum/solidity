contract C {
    function f() public {
        address payable a = payable(address(new D()));
        address payable b = payable(new E());
        address payable c = payable(new F());

        a;
        b;
        c;
    }
}

// A contract that cannot receive Ether
contract D {}

// A contract that can receive Ether
contract E {
    receive() external payable {
    }
}

// A contract that can receive Ether using the fallback
contract F {
    fallback() external payable {

    }
}

// ----
