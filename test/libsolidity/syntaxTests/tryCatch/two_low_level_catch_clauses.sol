contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch {
        } catch (bytes memory y) {
            y;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError: (112-161): This try statement already has a low-level catch clause.
