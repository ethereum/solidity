contract D {}
contract C {
    function foo(int a) payable external {
        this.foo{gas:2, gas: 5};
        this.foo{value:2, value: 5};
        this.foo{gas:2, value: 5, gas:2, value:3};
        new D{salt:"abc", salt:"efg"}();
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// TypeError 9886: (78-101): Duplicate option "gas".
// TypeError 9886: (111-138): Duplicate option "value".
// TypeError 9886: (148-189): Duplicate option "gas".
// TypeError 9886: (148-189): Duplicate option "value".
// TypeError 9886: (199-228): Duplicate option "salt".
