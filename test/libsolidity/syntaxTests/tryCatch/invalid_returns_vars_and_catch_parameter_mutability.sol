contract C {
    function f() public returns (uint, uint) {
        try this.f() returns (uint constant a, uint immutable b) {
        } catch Error(string memory immutable x) {
            x;
        }

        try this.f() returns (uint a, uint b) {
        } catch (bytes memory immutable x) {
            x;
        }

        try this.f() returns (uint a, uint b) {
        } catch Error(string memory constant x) {
            x;
        }

        try this.f() returns (uint a, uint b) {
        } catch (bytes memory constant x) {
            x;
        }
    }
}
// ----
// DeclarationError 1788: (90-105): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 8297: (107-123): The "immutable" keyword can only be used for state variables.
// DeclarationError 8297: (149-174): The "immutable" keyword can only be used for state variables.
// DeclarationError 8297: (269-293): The "immutable" keyword can only be used for state variables.
// DeclarationError 1788: (393-417): The "constant" keyword can only be used for state variables or variables at file level.
// DeclarationError 1788: (512-535): The "constant" keyword can only be used for state variables or variables at file level.
