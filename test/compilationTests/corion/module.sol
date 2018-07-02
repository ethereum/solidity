pragma solidity ^0.4.11;

contract abstractModuleHandler {
    function transfer(address from, address to, uint256 value, bool fee) external returns (bool success) {}
    function balanceOf(address owner) public view returns (bool success, uint256 value) {}
}

contract module {
    /*
        Module
    */
    
    enum status {
        New,
        Connected,
        Disconnected,
        Disabled
    }
    
    status public moduleStatus;
    uint256 public disabledUntil;
    address public moduleHandlerAddress;
    
    function disableModule(bool forever) external onlyForModuleHandler returns (bool success) {
        _disableModule(forever);
        return true;
    }
    function _disableModule(bool forever) internal {
        /*
            Disable the module for one week, if the forever true then for forever.
            This function calls the Publisher module.
            
            @forever    For forever or not
        */
        if ( forever ) { moduleStatus = status.Disabled; }
        else { disabledUntil = block.number + 5760; }
    }
    
    function replaceModuleHandler(address newModuleHandlerAddress) external onlyForModuleHandler returns (bool success) {
        _replaceModuleHandler(newModuleHandlerAddress);
        return true;
    }
    function _replaceModuleHandler(address newModuleHandlerAddress) internal {
        /*
            Replace the ModuleHandler address.
            This function calls the Publisher module.
            
            @newModuleHandlerAddress    New module handler address
        */
        require( moduleStatus == status.Connected );
        moduleHandlerAddress = newModuleHandlerAddress;
    }
    
    function connectModule() external onlyForModuleHandler returns (bool success) {
        _connectModule();
        return true;
    }
    function _connectModule() internal {
        /*
            Registering and/or connecting-to ModuleHandler.
            This function is called by ModuleHandler load or by Publisher.
        */
        require( moduleStatus == status.New );
        moduleStatus = status.Connected;
    }
    
    function disconnectModule() external onlyForModuleHandler returns (bool success) {
        _disconnectModule();
        return true;
    }
    function _disconnectModule() internal {
        /*
            Disconnect the module from the ModuleHandler.
            This function calls the Publisher module.
        */
        require( moduleStatus != status.New && moduleStatus != status.Disconnected );
        moduleStatus = status.Disconnected;
    }
    
    function replaceModule(address newModuleAddress) external onlyForModuleHandler returns (bool success) {
        _replaceModule(newModuleAddress);
        return true;
    }
    function _replaceModule(address newModuleAddress) internal {
        /*
            Replace the module for an another new module.
            This function calls the Publisher module.
            We send every Token and ether to the new module.
            
            @newModuleAddress   New module handler address
        */
        require( moduleStatus != status.New && moduleStatus != status.Disconnected);
        (bool _success, uint256 _balance) = abstractModuleHandler(moduleHandlerAddress).balanceOf(address(this));
        require( _success );
        if ( _balance > 0 ) {
            require( abstractModuleHandler(moduleHandlerAddress).transfer(address(this), newModuleAddress, _balance, false) );
        }
        if ( this.balance > 0 ) {
            require( newModuleAddress.send(this.balance) );
        }
        moduleStatus = status.Disconnected;
    }
    
    function transferEvent(address from, address to, uint256 value) external onlyForModuleHandler returns (bool success) {
        return true;
    }
    function newSchellingRoundEvent(uint256 roundID, uint256 reward) external onlyForModuleHandler returns (bool success) {
        return true;
    }
    
    function registerModuleHandler(address _moduleHandlerAddress) internal {
        /*
            Module constructor function for registering ModuleHandler address.
        */
        moduleHandlerAddress = _moduleHandlerAddress;
    }
    function isModuleHandler(address addr) internal returns (bool ret) {
        /*
            Test for ModuleHandler address.
            If the module is not connected then returns always false.
            
            @addr   Address to check
            
            @ret    This is the module handler address or not
        */
        if ( moduleHandlerAddress == address(0x00) ) { return true; }
        if ( moduleStatus != status.Connected ) { return false; }
        return addr == moduleHandlerAddress;
    }
    function isActive() public view returns (bool success, bool active) {
        /*
            Check self for ready for functions or not.
            
            @success    Function call was successfull or not
            @active     Ready for functions or not
        */
        return (true, moduleStatus == status.Connected && block.number >= disabledUntil);
    }
    modifier onlyForModuleHandler() {
        require( msg.sender == moduleHandlerAddress );
        _;
    }
}
