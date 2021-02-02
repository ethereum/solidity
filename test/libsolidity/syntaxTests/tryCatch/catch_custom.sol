contract C {
    error E();
    function f() public {
        try this.f() {

        } catch E() {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// UnimplementedFeatureError: NONE
