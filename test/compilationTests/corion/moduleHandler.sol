pragma solidity ^0.4.11;

import "./module.sol";
import "./announcementTypes.sol";
import "./multiOwner.sol";

import "./publisher.sol";
import "./token.sol";
import "./provider.sol";
import "./schelling.sol";
import "./premium.sol";
import "./ico.sol";

contract abstractModule {
    function connectModule() external returns (bool success) {}
    function disconnectModule() external returns (bool success) {}
    function replaceModule(address addr) external returns (bool success) {}
    function disableModule(bool forever) external returns (bool success) {}
    function isActive() public view returns (bool success) {}
    function replaceModuleHandler(address newHandler) external returns (bool success) {}
    function transferEvent(address from, address to, uint256 value) external returns (bool success) {}
    function newSchellingRoundEvent(uint256 roundID, uint256 reward) external returns (bool success) {}
}

contract moduleHandler is multiOwner, announcementTypes {
    
    struct modules_s {
        address addr;
        bytes32 name;
        bool schellingEvent;
        bool transferEvent;
    }
    
    modules_s[] public modules;
    address public foundationAddress;
    uint256 debugModeUntil = block.number + 1000000;


    constructor(address[] newOwners) multiOwner(newOwners) public {}
    function load(address foundation, bool forReplace, address Token, address Premium, address Publisher, address Schelling, address Provider) public {
        /*
            Loading modulest to ModuleHandler.
            
            This module can be called only once and only by the owner, if every single module and its database are already put on the blockchain.
            If forReaplace is true, than the ModuleHandler will be replaced. Before the publishing of its replace, the new contract must be already on the blockchain.
            
            @foundation     Address of foundation.
            @forReplace     Is it for replace or not. If not, it will be connected to the module.
            @Token          address of token.
            @Publisher      address of publisher.
            @Schelling      address of Schelling.
            @Provider       address of provider
        */
        require( owners[msg.sender] );
        require( modules.length == 0 );
        foundationAddress = foundation;
        addModule( modules_s(Token,      keccak256('Token'),      false, false),  ! forReplace);
        addModule( modules_s(Premium,    keccak256('Premium'),    false, false),  ! forReplace);
        addModule( modules_s(Publisher,  keccak256('Publisher'),  false, true),   ! forReplace);
        addModule( modules_s(Schelling,  keccak256('Schelling'),  false, true),   ! forReplace);
        addModule( modules_s(Provider,   keccak256('Provider'),   true, true),    ! forReplace);
    }
    function addModule(modules_s input, bool call) internal {
        /*
            Inside function for registration of the modules in the database.
            If the call is false, won't happen any direct call.
            
            @input  _Structure of module.
            @call   Is connect to the module or not.
        */
        if ( call ) { require( abstractModule(input.addr).connectModule() ); }
        (bool success, bool found, uint256 id) = getModuleIDByAddress(input.addr);
        require( success && ! found );
        (success, found, id) = getModuleIDByHash(input.name);
        require( success && ! found );
        (success, found, id) = getModuleIDByAddress(address(0x00));
        require( success );
        if ( ! found ) {
            id = modules.length;
            modules.length++;
        }
        modules[id] = input;
    }
    function getModuleAddressByName(string name) public view returns( bool success, bool found, address addr ) {
        /*
            Search by name for module. The result is an Ethereum address.
            
            @name       Name of module.
            @addr       Address of module.
            @found      Is there any result.
            @success    Was the transaction successful or not.
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByName(name);
        if ( _success && _found ) { return (true, true, modules[_id].addr); }
        return (true, false, address(0x00));
    }
    function getModuleIDByHash(bytes32 hashOfName) public view returns( bool success, bool found, uint256 id ) {
        /*
            Search by hash of name in the module array. The result is an index array.
            
            @name       Name of module.
            @id         Index of module.
            @found      Was there any result or not.
        */
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            if ( modules[a].name == hashOfName ) {
                return (true, true, a);
            }
        }
        return (true, false, 0);
    }
    function getModuleIDByName(string name) public view returns( bool success, bool found, uint256 id ) {
        /*
            Search by name for module. The result is an index array.
            
            @name       Name of module.
            @id         Index of module.
            @found      Was there any result or not.
        */
        bytes32 _name = keccak256(bytes(name));
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            if ( modules[a].name == _name ) {
                return (true, true, a);
            }
        }
        return (true, false, 0);
    }
    function getModuleIDByAddress(address addr) public view returns( bool success, bool found, uint256 id ) {
        /*
            Search by ethereum address for module. The result is an index array.
            
            @name       Name of module.
            @id         Index of module.
            @found      Was there any result or not.
        */
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            if ( modules[a].addr == addr ) {
                return (true, true, a);
            }
        }
        return (true, false, 0);
    }
    function replaceModule(string name, address addr, bool callCallback) external returns (bool success) {
        /*
            Module replace, can be called only by the Publisher contract.
            
            @name           Name of module.
            @addr           Address of module.
            @bool           Was there any result or not.
            @callCallback   Call the replaceable module to confirm replacement or not.
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success );
        if ( ! ( _found && modules[_id].name == keccak256('Publisher') )) {
            require( block.number < debugModeUntil );
            if ( ! insertAndCheckDo(calcDoHash("replaceModule", keccak256(abi.encodePacked(name, addr, callCallback)))) ) {
                return true;
            }
        }
        (_success, _found, _id) = getModuleIDByName(name);
        require( _success && _found );
        if ( callCallback ) {
            require( abstractModule(modules[_id].addr).replaceModule(addr) );
        }
        require( abstractModule(addr).connectModule() );
        modules[_id].addr = addr;
        return true;
    }
    
    function callReplaceCallback(string moduleName, address newModule) external returns (bool success) {
        require( block.number < debugModeUntil );
        if ( ! insertAndCheckDo(calcDoHash("callReplaceCallback", keccak256(abi.encodePacked(moduleName, newModule)))) ) {
            return true;
        }
        (bool _success, bool _found, uint256 _id) = getModuleIDByName(moduleName);
        require( _success);
        require( abstractModule(modules[_id].addr).replaceModule(newModule) );
        return true;
    }
    
    function newModule(string name, address addr, bool schellingEvent, bool transferEvent) external returns (bool success) {
        /*
            Adding new module to the database. Can be called only by the Publisher contract.
            
            @name               Name of module.
            @addr               Address of module.
            @schellingEvent     Gets it new Schelling round notification?
            @transferEvent      Gets it new transaction notification?
            @bool               Was there any result or not.
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success );
        if ( ! ( _found && modules[_id].name == keccak256('Publisher') )) {
            require( block.number < debugModeUntil );
            if ( ! insertAndCheckDo(calcDoHash("newModule", keccak256(abi.encodePacked(name, addr, schellingEvent, transferEvent)))) ) {
                return true;
            }
        }
        addModule( modules_s(addr, keccak256(bytes(name)), schellingEvent, transferEvent), true);
        return true;
    }
    function dropModule(string name, bool callCallback) external returns (bool success) {
        /*
            Deleting module from the database. Can be called only by the Publisher contract.
            
            @name           Name of module to delete.
            @bool           Was the function successful?
            @callCallback   Call the replaceable module to confirm replacement or not.
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success );
        if ( ! ( _found && modules[_id].name == keccak256('Publisher') )) {
            require( block.number < debugModeUntil );
            if ( ! insertAndCheckDo(calcDoHash("replaceModule", keccak256(abi.encodePacked(name, callCallback)))) ) {
                return true;
            }
        }
        (_success, _found, _id) = getModuleIDByName(name);
        require( _success && _found );
        if( callCallback ) {
            abstractModule(modules[_id].addr).disableModule(true);
        }
        delete modules[_id];
        return true;
    }
    
    function callDisableCallback(string moduleName) external returns (bool success) {
        require( block.number < debugModeUntil );
        if ( ! insertAndCheckDo(calcDoHash("callDisableCallback", keccak256(bytes(moduleName)))) ) {
            return true;
        }
        (bool _success, bool _found, uint256 _id) = getModuleIDByName(moduleName);
        require( _success);
        require( abstractModule(modules[_id].addr).disableModule(true) );
        return true;
    }
    
    function broadcastTransfer(address from, address to, uint256 value) external returns (bool success) {
        /*
            Announcing transactions for the modules.
            
            Can be called only by the token module.
            Only the configured modules get notifications.( transferEvent )
            
            @from       from who.
            @to         to who.
            @value      amount.
            @bool       Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success && _found && modules[_id].name == keccak256('Token') );
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            if ( modules[a].transferEvent && abstractModule(modules[a].addr).isActive() ) {
                require( abstractModule(modules[a].addr).transferEvent(from, to, value) );
            }
        }
        return true;
    }
    function broadcastSchellingRound(uint256 roundID, uint256 reward) external returns (bool success) {
        /*
            Announcing new Schelling round for the modules.
            Can be called only by the Schelling module.
            Only the configured modules get notifications( schellingEvent ).
            
            @roundID        Number of Schelling round.
            @reward         Coin emission in this Schelling round.
            @bool           Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success && _found && modules[_id].name == keccak256('Schelling') );
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            if ( modules[a].schellingEvent && abstractModule(modules[a].addr).isActive() ) {
                require( abstractModule(modules[a].addr).newSchellingRoundEvent(roundID, reward) );
            }
        }
        return true;
    }
    function replaceModuleHandler(address newHandler) external returns (bool success) {
        /*
            Replacing ModuleHandler.
            
            Can be called only by the publisher.
            Every module will be informed about the ModuleHandler replacement.
            
            @newHandler     Address of the new ModuleHandler.
            @bool           Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success );
        if ( ! ( _found && modules[_id].name == keccak256('Publisher') )) {
            require( block.number < debugModeUntil );
            if ( ! insertAndCheckDo(calcDoHash("replaceModuleHandler", keccak256(abi.encodePacked(newHandler)))) ) {
                return true;
            }
        }
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            require( abstractModule(modules[a].addr).replaceModuleHandler(newHandler) );
        }
        return true;
    }
    function balanceOf(address owner) public view returns (bool success, uint256 value) {
        /*
            Query of token balance.
            
            @owner     address
            @value      balance.
            @success    was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByName('Token');
        require( _success && _found );
        return (true, token(modules[_id].addr).balanceOf(owner));
    }
    function totalSupply() public view returns (bool success, uint256 value) {
        /*
            Query of the whole token amount.
            
            @value      amount.
            @success    was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByName('Token');
        require( _success && _found );
        return (true, token(modules[_id].addr).totalSupply());
    }
    function isICO() public view returns (bool success, bool ico) {
        /*
            Query of ICO state
            
            @ico        Is ICO in progress?.
            @success    was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByName('Token');
        require( _success && _found );
        return (true, token(modules[_id].addr).isICO());
    }
    function getCurrentSchellingRoundID() public view returns (bool success, uint256 round) {
        /*
            Query of number of the actual Schelling round.
            
            @round      Schelling round.
            @success    was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByName('Schelling');
        require( _success && _found );
        return (true, schelling(modules[_id].addr).getCurrentSchellingRoundID());
    }
    function mint(address to, uint256 value) external returns (bool success) {
        /*
            Token emission request. Can be called only by the provider.
            
            @to         Place of new token
            @value      Token amount
            
            @success    Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success && _found && modules[_id].name == keccak256('Provider') );
        (_success, _found, _id) = getModuleIDByName('Token');
        require( _success && _found );
        require( token(modules[_id].addr).mint(to, value) );
        return true;
    }
    function transfer(address from, address to, uint256 value, bool fee) external returns (bool success) {
        /*
            Token transaction request. Can be called only by a module.
            
            @from       from who.
            @to         To who.
            @value      Token amount.
            @fee        Transaction fee will be charged or not?
            @success    Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success && _found );
        (_success, _found, _id) = getModuleIDByName('Token');
        require( _success && _found );
        require( token(modules[_id].addr).transferFromByModule(from, to, value, fee) );
        return true;
    }
    function processTransactionFee(address from, uint256 value) external returns (bool success) {
        /*
            Token transaction fee. Can be called only by the provider.
            
            @from       From who.
            @value      Token amount.
            @success    Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success && _found && modules[_id].name == keccak256('Provider') );
        (_success, _found, _id) = getModuleIDByName('Token');
        require( _success && _found );
        require( token(modules[_id].addr).processTransactionFee(from, value) );
        return true;
    }
    function burn(address from, uint256 value) external returns (bool success) {
        /*
            Token burn. Can be called only by Schelling.
            
            @from       From who.
            @value      Token amount.
            @success    Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success && _found && modules[_id].name == keccak256('Schelling') );
        (_success, _found, _id) = getModuleIDByName('Token');
        require( _success && _found );
        require( token(modules[_id].addr).burn(from, value) );
        return true;
    }
    function configureModule(string moduleName, announcementType aType, uint256 value) external returns (bool success) {
        /*
            Changing configuration of a module. Can be called only by Publisher or while debug mode by owners.
            
            @moduleName Module name which will be configured
            @aType      Type of variable (announcementType).
            @value      New value
            @success    Was the function successful?
        */
        (bool _success, bool _found, uint256 _id) = getModuleIDByAddress(msg.sender);
        require( _success );
        if ( ! ( _found && modules[_id].name == keccak256('Publisher') )) {
            require( block.number < debugModeUntil );
            if ( ! insertAndCheckDo(calcDoHash("configureModule", keccak256(abi.encodePacked(moduleName, aType, value)))) ) {
                return true;
            }
        }
        (_success, _found, _id) = getModuleIDByName(moduleName);
        require( _success && _found );
        require( schelling(modules[_id].addr).configure(aType, value) );
        return true;
    }
    function freezing(bool forever) external {
        /*
            Freezing CORION Platform. Can be called only by the owner.
            Freeze can not be recalled!
            
            @forever    Is it forever or not?
        */
        require( owners[msg.sender] );
        if ( forever ) {
            if ( ! insertAndCheckDo(calcDoHash("freezing", keccak256(abi.encodePacked(forever)))) ) {
                return;
            }            
        }
        for ( uint256 a=0 ; a<modules.length ; a++ ) {
            require( abstractModule(modules[a].addr).disableModule(forever) );
        }
    }
}
