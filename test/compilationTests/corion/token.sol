pragma solidity ^0.4.11;

import "./announcementTypes.sol";
import "./safeMath.sol";
import "./module.sol";
import "./moduleHandler.sol";
import "./tokenDB.sol";

contract thirdPartyContractAbstract {
    function receiveCorionToken(address, uint256, bytes) external returns (bool, uint256) {}
    function approvedCorionToken(address, uint256, bytes) external returns (bool) {}
}

contract token is safeMath, module, announcementTypes {
    /*
        module callbacks
    */
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
    * @title Corion Platform Token
    * @author iFA @ Corion Platform
    *
    */
    string public name = "Corion";
    string public symbol = "COR";
    uint8 public decimals = 6;
    
    tokenDB public db;
    address public icoAddr;
    uint256 public transactionFeeRate      = 20;
    uint256 public transactionFeeRateM     = 1e3;
    uint256 public transactionFeeMin       =   20000;
    uint256 public transactionFeeMax       = 5000000;
    uint256 public transactionFeeBurn      = 80;
    address public exchangeAddress;
    bool    public isICO                   = true;
    
    mapping(address => bool) public genesis;

    constructor(bool forReplace, address moduleHandler, address dbAddr, address icoContractAddr, address exchangeContractAddress, address[] genesisAddr, uint256[] genesisValue) public payable {
        /*
            Installation function
            
            When _icoAddr is defined, 0.2 ether has to be attached  as many times  as many genesis addresses are given
            
            @forReplace                 This address will be replaced with the old one or not.
            @moduleHandler              Modulhandler's address
            @dbAddr                     Address of database
            @icoContractAddr            Address of ICO contract
            @exchangeContractAddress    Address of Market in order to buy gas during ICO
            @genesisAddr                Array of Genesis addresses
            @genesisValue               Array of balance of genesis addresses
        */
        super.registerModuleHandler(moduleHandler);
        require( dbAddr != address(0x00) );
        require( icoContractAddr != address(0x00) );
        require( exchangeContractAddress != address(0x00) );
        db = tokenDB(dbAddr);
        icoAddr = icoContractAddr;
        exchangeAddress = exchangeContractAddress;
        isICO = ! forReplace;
        if ( ! forReplace ) {
            require( db.replaceOwner(this) );
            assert( genesisAddr.length == genesisValue.length );
            require( address(this).balance >= genesisAddr.length * 0.2 ether );
            for ( uint256 a=0 ; a<genesisAddr.length ; a++ ) {
                genesis[genesisAddr[a]] = true;
                require( db.increase(genesisAddr[a], genesisValue[a]) );
                if ( ! genesisAddr[a].send(0.2 ether) ) {}
                emit Mint(genesisAddr[a], genesisValue[a]);
            }
        }
    }
    
    function closeIco() external returns (bool success) {
        /*
            ICO finished. It can be called only by ICO contract
            
            @success    Was the Function successful?
        */
        require( msg.sender == icoAddr );
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
            Authorise another address to use a certain quantity of the authorising owner’s balance
         
            @spender            Address of authorised party
            @amount             Token quantity
            @nonce              Transaction count
            
            @success            Was the Function successful?
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
            Authorise another address to use a certain quantity of the authorising  owner’s balance
            Following the transaction the receiver address `approvedCorionToken` function is called by the given data
            
            @spender            Authorized address
            @amount             Token quantity
            @extraData          Extra data to be received by the receiver
            @nonce              Transaction count
            
            @success            Was the Function successful?
        */
        _approve(spender, amount, nonce);
        require( thirdPartyContractAbstract(spender).approvedCorionToken(msg.sender, amount, extraData) );
        return true;
    }
    
    function _approve(address spender, uint256 amount, uint256 nonce) internal {
        /*
            Internal Function to authorise another address to use a certain quantity of the authorising owner’s balance.
            If the transaction count not match the authorise fails.
            
            @spender           Address of authorised party
            @amount            Token quantity
            @nonce             Transaction count
        */
        require( msg.sender != spender );
        require( db.balanceOf(msg.sender) >= amount );
        require( db.setAllowance(msg.sender, spender, amount, nonce) );
        emit Approval(msg.sender, spender, amount);
    }

    function allowance(address owner, address spender) public view returns (uint256 remaining, uint256 nonce) {
        /*
            Get the quantity of tokens given to be used
            
            @owner         Authorising address
            @spender       Authorised address
            
            @remaining     Tokens to be spent
            @nonce         Transaction count
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
            Start transaction, token is sent from caller’s address to receiver’s address
            Transaction fee is to be deducted.
            If receiver is not a natural address but a person, he will be called
          
            @to         To who
            @amount     Quantity
            
            @success    Was the Function successful?
        */
        bytes memory _data;
        if ( isContract(to) ) {
            _transferToContract(msg.sender, to, amount, _data);
        } else {
            _transfer( msg.sender, to, amount, true);
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
            Start transaction to send a quantity from a given address to another address. (approve / allowance). This can be called only by the address approved in advance
            Transaction fee is to be deducted
            If receiver is not a natural address but a person, he will be called
            
            @from       From who.
            @to         To who
            @amount     Quantity
            
            @success    Was the Function successful?
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
            _transferToContract(from, to, amount, _data);
        } else {
            _transfer( from, to, amount, true);
        }
        emit Transfer(from, to, amount, _data);
        return true;
    }
    
    /**
     * @notice Send `amount` tokens to `to` from `from` on the condition it is approved by `from`
     * @param from The address holding the tokens being transferred
     * @param to The address of the recipient
     * @param amount The amount of tokens to be transferred
     * @return True if the transfer was successful
     */
    function transferFromByModule(address from, address to, uint256 amount, bool fee) isReady external returns (bool success) {
        /*
            Start transaction to send a quantity from a given address to another address
            Only ModuleHandler can call it
           
            @from       From who
            @to         To who.
            @amount     Quantity
            @fee        Deduct transaction fee - yes or no?
            
            @success    Was the Function successful?
        */
        bytes memory _data;
        require( super.isModuleHandler(msg.sender) );
        _transfer( from, to, amount, fee);
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
            Start transaction to send a quantity from a given address to another address
            After transaction the function `receiveCorionToken`of the receiver is called  by the given data
            When sending an amount, it is possible the total amount cannot be processed, the remaining amount is sent back with no fee charged
            
            @to             To who.
            @amount         Quantity
            @extraData      Extra data the receiver will get
            
            @success        Was the Function successful?
        */
        if ( isContract(to) ) {
            _transferToContract(msg.sender, to, amount, extraData);
        } else {
            _transfer( msg.sender, to, amount, true);
        }
        emit Transfer(msg.sender, to, amount, extraData);
        return true;
    }
    
    function _transferToContract(address from, address to, uint256 amount, bytes extraData) internal {
        /*
            Internal function to start transactions to a contract
            
            @from           From who
            @to             To who.
            @amount         Quantity
            @extraData      Extra data the receiver will get
        */
        _transfer(from, to, amount, exchangeAddress == to);
        (bool _success, uint256 _back) = thirdPartyContractAbstract(to).receiveCorionToken(from, amount, extraData);
        require( _success );
        require( amount > _back );
        if ( _back > 0 ) {
            _transfer(to, from, _back, false);
        }
        _processTransactionFee(from, amount - _back);
    }
    
    function _transfer(address from, address to, uint256 amount, bool fee) internal {
        /*
            Internal function to start transactions. When Tokens are sent, transaction fee is charged
            During ICO transactions are allowed only from genesis addresses.
            After sending the tokens, the ModuleHandler is notified and it will broadcast the fact among members 
            
            The 0xa636a97578d26a3b76b060bbc18226d954cf3757 address are blacklisted.
            
            @from       From who
            @to         To who
            @amount     Quantity
            @fee        Deduct transaction fee - yes or no?
        */
        if( fee ) {
            (bool success, uint256 _fee) = getTransactionFee(amount);
            require( success );
            require( db.balanceOf(from) >= amount + _fee );
        }
        require( from != address(0x00) && to != address(0x00) && to != 0xa636A97578d26A3b76B060Bbc18226d954cf3757 );
        require( ( ! isICO) || genesis[from] );
        require( db.decrease(from, amount) );
        require( db.increase(to, amount) );
        if ( fee ) { _processTransactionFee(from, amount); }
        if ( isICO ) {
            require( ico(icoAddr).setInterestDB(from, db.balanceOf(from)) );
            require( ico(icoAddr).setInterestDB(to, db.balanceOf(to)) );
        }
        require( moduleHandler(moduleHandlerAddress).broadcastTransfer(from, to, amount) );
    }
    
    /**
     * @notice Transaction fee will be deduced from `owner` for transacting `value`
     * @param owner The address where will the transaction fee deduced
     * @param value The base for calculating the fee
     * @return True if the transfer was successful
     */
    function processTransactionFee(address owner, uint256 value) isReady external returns (bool success) {
        /*
            Charge transaction fee. It can be called only by moduleHandler  
        
            @owner      From who.
            @value      Quantity to calculate the fee
            
            @success    Was the Function successful?
        */
        require( super.isModuleHandler(msg.sender) );
        _processTransactionFee(owner, value);
        return true;
    }
    
    function _processTransactionFee(address owner, uint256 value) internal {
        /*
            Internal function to charge the transaction fee. A certain quantity is burnt, the rest is sent to the Schelling game prize pool.
            No transaction fee during ICO.
            
            @owner      From who
            @value      Quantity to calculate the fee
        */
        if ( isICO ) { return; }
        (bool _success, uint256 _fee) = getTransactionFee(value);
        require( _success );
        uint256 _forBurn = _fee * transactionFeeBurn / 100;
        uint256 _forSchelling = _fee - _forBurn;
        bool _found;
        address _schellingAddr;
        (_success, _found, _schellingAddr) = moduleHandler(moduleHandlerAddress).getModuleAddressByName('Schelling');
        require( _success );
        if ( _schellingAddr != address(0x00) && _found) {
            require( db.decrease(owner, _forSchelling) );
            require( db.increase(_schellingAddr, _forSchelling) );
            _burn(owner, _forBurn);
            bytes memory _data;
            emit Transfer(owner, _schellingAddr, _forSchelling, _data);
            require( moduleHandler(moduleHandlerAddress).broadcastTransfer(owner, _schellingAddr, _forSchelling) );
        } else {
            _burn(owner, _fee);
        }
    }
    
    function getTransactionFee(uint256 value) public view returns (bool success, uint256 fee) {
        /*
            Transaction fee query.
            
            @value      Quantity to calculate the fee
            
            @success    Was the Function successful?
            @fee        Amount of Transaction fee
        */
        if ( isICO ) { return (true, 0); }
        fee = value * transactionFeeRate / transactionFeeRateM / 100;
        if ( fee > transactionFeeMax ) { fee = transactionFeeMax; }
        else if ( fee < transactionFeeMin ) { fee = transactionFeeMin; }
        return (true, fee);
    }
    
    function mint(address owner, uint256 value) isReady external returns (bool success) {
        /*
            Generating tokens. It can be called only by ICO contract or the moduleHandler.
            
            @owner      Address
            @value      Amount.
            
            @success    Was the Function successful?
        */
        require( super.isModuleHandler(msg.sender) || msg.sender == icoAddr );
        _mint(owner, value);
        return true;
    }
    
    function _mint(address owner, uint256 value) internal {
        /*
            Internal function to generate tokens
            
            @owner     Token is credited to this address
            @value     Quantity
        */
        require( db.increase(owner, value) );
        require( moduleHandler(moduleHandlerAddress).broadcastTransfer(address(0x00), owner, value) );
        if ( isICO ) {
            require( ico(icoAddr).setInterestDB(owner, db.balanceOf(owner)) );
        }
        emit Mint(owner, value);
    }
    
    function burn(address owner, uint256 value) isReady external returns (bool success) {
        /*
            Burning the token. Can call only modulehandler
            
            @owner     Burn the token from this address
            @value     Quantity
            
            @success    Was the Function successful?
        */
        require( super.isModuleHandler(msg.sender) );
        _burn(owner, value);
        return true;
    }
    
    function _burn(address owner, uint256 value) internal {
        /*
            Internal function to burn the token
     
            @owner     Burn the token from this address
            @value     Quantity
        */
        require( db.decrease(owner, value) );
        require( moduleHandler(moduleHandlerAddress).broadcastTransfer(owner, address(0x00), value) );
        emit Burn(owner, value);
    }
    
    function isContract(address addr) internal returns (bool success) {
        /*
            Internal function to check if the given address is natural, or a contract
            
            @addr       Address to be checked
            
            @success    Is the address crontact or not
        */
        uint256 _codeLength;
        assembly {
            _codeLength := extcodesize(addr)
        }
        return _codeLength > 0;
    }

    function balanceOf(address owner) public view returns (uint256 value) {
        /*
            Token balance query
            
            @owner      Address
            
            @value      Balance of address
        */
        return db.balanceOf(owner);
    }

    function totalSupply() public view returns (uint256 value) {
        /*
            Total token quantity query
            
            @value      Total token quantity
        */
        return db.totalSupply();
    }
    
    function configure(announcementType aType, uint256 value) isReady external returns(bool success) {
        /*
            Token settings configuration.It  can be call only by moduleHandler
           
            @aType      Type of setting
            @value      Value
            
            @success    Was the Function successful?
        */
        require( super.isModuleHandler(msg.sender) );
        if      ( aType == announcementType.transactionFeeRate )    { transactionFeeRate = value; }
        else if ( aType == announcementType.transactionFeeMin )     { transactionFeeMin = value; }
        else if ( aType == announcementType.transactionFeeMax )     { transactionFeeMax = value; }
        else if ( aType == announcementType.transactionFeeBurn )    { transactionFeeBurn = value; }
        else { return false; }
        return true;
    }
    
    event AllowanceUsed(address indexed spender, address indexed owner, uint256 indexed value);
    event Mint(address indexed addr, uint256 indexed value);
    event Burn(address indexed addr, uint256 indexed value);
    event Approval(address indexed _owner, address indexed _spender, uint256 _value);
    event Transfer(address indexed _from, address indexed _to, uint256 indexed _value, bytes _extraData);
}
