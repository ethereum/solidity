pragma solidity ^0.4.11;

import "./safeMath.sol";
import "./tokenDB.sol";
import "./module.sol";

contract thirdPartyPContractAbstract {
    function receiveCorionPremiumToken(address, uint256, bytes) external returns (bool, uint256) {}
    function approvedCorionPremiumToken(address, uint256, bytes) external returns (bool) {}
}

contract ptokenDB is tokenDB {}

contract premium is module, safeMath {
    function replaceModule(address addr) external returns (bool success) {
        require( super.isModuleHandler(msg.sender) );
        require( db.replaceOwner(addr) );
        super._replaceModule(addr);
        return true;
    }
    modifier isReady {
        (bool _success, bool _active) = super.isActive();
        require( _success && _active ); 
        _;
    }
    /**
    *
    * @title Corion Platform Premium Token
    * @author iFA @ Corion Platform
    *
    */
    
    string public name = "Corion Premium";
    string public symbol = "CORP";
    uint8 public decimals = 0;
    
    address public icoAddr;
    tokenDB public db;
    bool    public  isICO;
    
    mapping(address => bool) public genesis;
    
    constructor(bool forReplace, address moduleHandler, address dbAddress, address icoContractAddr, address[] genesisAddr, uint256[] genesisValue) {
        /*
            Setup function.
            If an ICOaddress is defined then the balance of the genesis addresses will be set as well.
            
            @forReplace         This address will be replaced with the old one or not.
            @moduleHandler      Modulhandler’s address
            @dbAddress          Address of database
            @icoContractAddr    Address of ico contract.
            @genesisAddr        Array of the genesis addresses.
            @genesisValue       Array of the balance of the genesis addresses
        */
        super.registerModuleHandler(moduleHandler);
        require( dbAddress != address(0x00) );
        db = ptokenDB(dbAddress);
        if ( ! forReplace ) {
            require( db.replaceOwner(this) );
            isICO = true;
            icoAddr = icoContractAddr;
            assert( genesisAddr.length == genesisValue.length );
            for ( uint256 a=0 ; a<genesisAddr.length ; a++ ) {
                genesis[genesisAddr[a]] = true;
                require( db.increase(genesisAddr[a], genesisValue[a]) );
                emit Mint(genesisAddr[a], genesisValue[a]);
            }
        }
    }
    
    function closeIco() external returns (bool success) {
        /*
            Finishing the ICO. Can be invited only by an ICO contract.
            
            @success    If the function was successful.
        */
        require( isICO );
        isICO = false;
        return true;
    }
    
    /**
     * @notice `msg.sender` approves `spender` to spend `amount` tokens on its behalf.
     * @param spender The address of the account able to transfer the tokens
     * @param amount The amount of tokens to be approved for transfer
     * @param nonce The transaction count of the authorised address
     * @return True if the approval was successful
     */
    function approve(address spender, uint256 amount, uint256 nonce) isReady external returns (bool success) {
        /*
            Authorize another address to use an exact amount of the principal’s balance.   
            
            @spender    Address of authorised party
            @amount     Token quantity
            @nonce      Transaction count
            
            @success    Was the Function successful?
        */
        _approve(spender, amount, nonce);
        return true;
    }
    
    /**
     * @notice `msg.sender` approves `spender` to spend `amount` tokens on its behalf and notify the spender from your approve with your `extraData` data.
     * @param spender The address of the account able to transfer the tokens
     * @param amount The amount of tokens to be approved for transfer
     * @param nonce The transaction count of the authorised address
     * @param extraData Data to give forward to the receiver
     * @return True if the approval was successful
     */
    function approveAndCall(address spender, uint256 amount, uint256 nonce, bytes extraData) isReady external returns (bool success) {
        /*
            Authorize another address to use an exact amount of the principal’s balance.
            After the transaction the approvedCorionPremiumToken function of the address will be called with the given data.
            
            @spender        Authorized address
            @amount         Token quantity
            @extraData      Extra data to be received by the receiver
            @nonce          Transaction count
            
            @sucess         Was the Function successful?
        */
        _approve(spender, amount, nonce);
        require( thirdPartyPContractAbstract(spender).approvedCorionPremiumToken(msg.sender, amount, extraData) );
        return true;
    }
    
    function _approve(address spender, uint256 amount, uint256 nonce) isReady internal {
        /*
            Inner function to authorize another address to use an exact amount of the principal’s balance. 
            If the transaction count not match the authorise fails.
            
            @spender    Address of authorised party
            @amount     Token quantity
            @nonce      Transaction count
        */
        require( msg.sender != spender );
        require( db.balanceOf(msg.sender) >= amount );
        require( db.setAllowance(msg.sender, spender, amount, nonce) );
        emit Approval(msg.sender, spender, amount);
    }
    
    function allowance(address owner, address spender) view returns (uint256 remaining, uint256 nonce) {
        /*
            Get the quantity of tokens given to be used
            
            @owner          Authorising address
            @spender        Authorised address
            
            @remaining      Tokens to be spent
            @nonce          Transaction count
        */
        (bool _success, uint256 _remaining, uint256 _nonce) = db.getAllowance(owner, spender);
        require( _success );
        return (_remaining, _nonce);
    }
    
    /**
     * @notice Send `amount` Corion tokens to `to` from `msg.sender`
     * @param to The address of the recipient
     * @param amount The amount of tokens to be transferred
     * @return Whether the transfer was successful or not
     */
    function transfer(address to, uint256 amount) isReady external returns (bool success) {
        /*
            Launch a transaction where the token is sent from the sender’s address to the receiver’s address.
            Transaction fee is going to be added as well.
            If the receiver is not a natural address but also a person then she/he will be invited as well.
            
            @to         For who
            @amount     Amount
            
            @success    Was the function successful?
        */
        bytes memory _data;
        if ( isContract(to) ) {
            transferToContract(msg.sender, to, amount, _data);
        } else {
            _transfer(msg.sender, to, amount);
        }
        emit Transfer(msg.sender, to, amount, _data);
        return true;
    }
    
    /**
     * @notice Send `amount` tokens to `to` from `from` on the condition it is approved by `from`
     * @param from The address holding the tokens being transferred
     * @param to The address of the recipient
     * @param amount The amount of tokens to be transferred
     * @return True if the transfer was successful
     */
    function transferFrom(address from, address to, uint256 amount) isReady external returns (bool success) {
        /*
            Launch a transaction where we transfer from a given address to another one. It can only be called by an address which was allowed before.
            Transaction fee will be charged too.
            If the receiver is not a natural address but also a person then she/he will be invited as well
            
            @from       From who?
            @to         For who?
            @amount     Amount
            
            @success    If the function was successful.
        */
        if ( from != msg.sender ) {
            (bool _success, uint256 _reamining, uint256 _nonce) = db.getAllowance(from, msg.sender);
            require( _success );
            _reamining = safeSub(_reamining, amount);
            _nonce = safeAdd(_nonce, 1);
            require( db.setAllowance(from, msg.sender, _reamining, _nonce) );
            emit AllowanceUsed(msg.sender, from, amount);
        }
        bytes memory _data;
        if ( isContract(to) ) {
            transferToContract(from, to, amount, _data);
        } else {
            _transfer( from, to, amount);
        }
        emit Transfer(from, to, amount, _data);
        return true;
    }
    
    /**
     * @notice Send `amount` Corion tokens to `to` from `msg.sender` and notify the receiver from your transaction with your `extraData` data
     * @param to The contract address of the recipient
     * @param amount The amount of tokens to be transferred
     * @param extraData Data to give forward to the receiver
     * @return Whether the transfer was successful or not
     */
    function transfer(address to, uint256 amount, bytes extraData) isReady external returns (bool success) {
        /*
            Launch a transaction where we transfer from a given address to another one.
            After thetransaction the approvedCorionPremiumToken function of the receiver’s address is going to be called with the given data.
            
            @to         For who?
            @amount     Amount
            @extraData  Extra data that will be given to the receiver
            
            @success    If the function was successful.
        */
        if ( isContract(to) ) {
            transferToContract(msg.sender, to, amount, extraData);
        } else {
            _transfer( msg.sender, to, amount);
        }
        emit Transfer(msg.sender, to, amount, extraData);
        return true;
    }
    
    function transferToContract(address from, address to, uint256 amount, bytes extraData) internal {
        /*
            Inner function in order to transact a contract.
            
            @to             For who?
            @amount         Amount
            @extraData      Extra data that will be given to the receiver
        */
        _transfer(from, to, amount);
        (bool _success, uint256 _back) = thirdPartyPContractAbstract(to).receiveCorionPremiumToken(from, amount, extraData);
        require( _success );
        require( amount > _back );
        if ( _back > 0 ) {
            _transfer(to, from, _back);
        }
    }
    
    function _transfer(address from, address to, uint256 amount) isReady internal {
        /*
            Inner function to launch a transaction.
            During the ICO transactions are only possible from the genesis address.
            0xa636a97578d26a3b76b060bbc18226d954cf3757 address is blacklisted.
            
            @from      From how?
            @to        For who?
            @amount    Amount
        */
        require( from != address(0x00) && to != address(0x00) && to != 0xa636A97578d26A3b76B060Bbc18226d954cf3757 );
        require( ( ! isICO) || genesis[from] );
        require( db.decrease(from, amount) );
        require( db.increase(to, amount) );
    }
    
    function mint(address owner, uint256 value) external returns (bool success) {
        /*
            Generating tokens. It can be called only by ICO contract.
            
            @owner      Address
            @value      Amount.
            
            @success    Was the Function successful?
        */
        require( msg.sender == icoAddr || isICO );
        _mint(owner, value);
        return true;
    }
    
    function _mint(address owner, uint256 value) isReady internal {
        /*
            Inner function to create a token.
            
            @owner     Address of crediting the token.
            @value     Amount
        */
        require( db.increase(owner, value) );
        emit Mint(owner, value);
    }
    
    function isContract(address addr) internal returns (bool success) {
        /*
            Inner function in order to check if the given address is a natural address or a contract.
            
            @addr       The address which is needed to be checked.
            
            @success    Is the address crontact or not
        */
        uint256 _codeLength;
        assembly {
            _codeLength := extcodesize(addr)
        }
        return _codeLength > 0;
    }
    
    function balanceOf(address owner) view returns (uint256 value) {
        /*
            Token balance query
            
            @owner      Address
            @value      Balance of address
        */
        return db.balanceOf(owner);
    }
    
    function totalSupply() view returns (uint256 value) {
        /*
            Total token quantity query
            
            @value      Total token quantity
        */
        return db.totalSupply();
    }
    
    event AllowanceUsed(address indexed spender, address indexed owner, uint256 indexed value);
    event Mint(address indexed addr, uint256 indexed value);
    event Burn(address indexed addr, uint256 indexed value);
    event Approval(address indexed _owner, address indexed _spender, uint256 _value);
    event Transfer(address indexed _from, address indexed _to, uint256 _value, bytes _extraData);
}
