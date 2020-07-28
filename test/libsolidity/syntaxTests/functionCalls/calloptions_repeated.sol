contract D {}
contract C {
    function foo(int a) payable external {
        this.foo{value:2, gas: 5}{gas:2};
        (this.foo{value:2, gas: 5}){gas:2};
        this.foo{value:2, gas: 5}{value:6};
        this.foo{gas:2, value: 5}{value:2, gas:5};
        new D{salt:"abc"}{salt:"a"}();
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// TypeError 9886: (78-110): Option "gas" has already been set.
// TypeError 9886: (120-154): Option "gas" has already been set.
// TypeError 9886: (164-198): Option "value" has already been set.
// TypeError 9886: (208-249): Option "value" has already been set.
// TypeError 9886: (208-249): Option "gas" has already been set.
// TypeError 9886: (259-286): Option "salt" has already been set.
