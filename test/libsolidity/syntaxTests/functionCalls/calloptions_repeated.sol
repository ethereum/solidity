contract D {}
contract C {
    function foo(int a) payable external {
        this.foo{value:2, gas: 5}{gas:2};
        (this.foo{value:2, gas: 5}){gas:2};
        this.foo{value:2, gas: 5}{value:6};
        this.foo.value(4){value:2, gas: 5};
        this.foo{gas:2, value: 5}{value:2, gas:5};
        new D{salt:"abc"}{salt:"a"}();
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// TypeError: (78-110): Option "gas" has already been set.
// TypeError: (120-154): Option "gas" has already been set.
// TypeError: (164-198): Option "value" has already been set.
// TypeError: (208-242): Option "value" has already been set.
// TypeError: (252-293): Option "value" has already been set.
// TypeError: (252-293): Option "gas" has already been set.
// TypeError: (303-330): Option "salt" has already been set.
