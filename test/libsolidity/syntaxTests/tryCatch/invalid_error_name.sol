contract C {
    function f() public returns (uint, uint) {
        try this.f() {
        } catch Error2() {
        } catch abc() {
        }
    }
}
// ----
// TypeError 3542: (93-119='catch Error2() {         }'): Invalid catch clause name. Expected either `catch (...)`, `catch Error(...)`, or `catch Panic(...)`.
// TypeError 3542: (120-143='catch abc() {         }'): Invalid catch clause name. Expected either `catch (...)`, `catch Error(...)`, or `catch Panic(...)`.
