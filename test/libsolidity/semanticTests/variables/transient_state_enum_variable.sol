contract C {
    enum Pets { Dog, Cat, Bird, Fish }
    Pets transient myPet;

    function f() public {
        myPet = Pets.Bird;
        this.g();
        assert(myPet == Pets.Cat);
    }
    function g() public {
        myPet = Pets.Cat;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() ->
