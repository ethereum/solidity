interface IBase {
    function foo() external view;
}

contract Base is IBase {
    function foo() public virtual view {}
}

interface IExt is IBase {}

contract Ext is IExt, Base {}

contract Impl is Ext {
    function foo() public view {}
}
// ----
// TypeError 9456: (211-240='function foo() public view {}'): Overriding function is missing "override" specifier.
// TypeError 4327: (211-240='function foo() public view {}'): Function needs to specify overridden contracts "Base" and "IBase".
