#######
通用模式
#######

*******
访问限制
*******

访问限制是合约的一种通用模式，但你不能限制任何人获取你的合约和交易的状态。当然，你可以通过加密来增加读取难度，但是如果你的合约需要读取该数据（指加密的数据），其他人也可以读取。

你可以通过将合约状态设置为私有来限制其他合约来读取你的合约状态。

此外，你可以限制其他人修改你的合约状态或者调用你的合约函数，这也是本章将要讨论的。

函数修饰符的使用可以让这些限制（访问限制）具有较好的可读性。

::
    contract AccessRestriction {
        // These will be assigned at the construction
        // phase, where `msg.sender` is the account
        // creating this contract.
        //以下变量将在构造函数中赋值 
        //msg.sender是你的账户
        //创建本合约
        address public owner = msg.sender;
        uint public creationTime = now;

        // Modifiers can be used to change
        // the body of a function.
        // If this modifier is used, it will
        // prepend a check that only passes
        // if the function is called from
        // a certain address.
        //修饰符可以用来修饰函数体，如果使用该修饰符，当该函数被其他地址调用时将会先检查是否允许调用（译注：就是说外部要调用本合约有修饰符的函数时会检查是否允许调用，比如该函数是私有的则外部不能调用。）

        modifier onlyBy(address _account)
        {
            if (msg.sender != _account)
                throw;
            // Do not forget the "_"! It will
            // be replaced by the actual function
            // body when the modifier is invoked.
            //account变量不要忘了“_” 
            _
        }

        /// Make `_newOwner` the new owner of this
        /// contract.
        //修改当前合约的宿主
        function changeOwner(address _newOwner)
            onlyBy(owner)
        {
            owner = _newOwner;
        }

        modifier onlyAfter(uint _time) {
            if (now < _time) throw;
            _
        }

        /// Erase ownership information.
        /// May only be called 6 weeks after
        /// the contract has been created.
        //清除宿主信息。只能在合约创建6周后调用
        function disown()
            onlyBy(owner)
            onlyAfter(creationTime + 6 weeks)
        {
            delete owner;
        }

        // This modifier requires a certain
        // fee being associated with a function call.
        // If the caller sent too much, he or she is
        // refunded, but only after the function body.
        // This is dangerous, because if the function
        // uses `return` explicitly, this will not be
        // done!
        //该修饰符和函数调用关联时需要消耗一部分费用。调用者发送的多余费用会在函数执行完成后返还，但这个是相当危险的，因为如果函数
        modifier costs(uint _amount) {
            if (msg.value < _amount)
                throw;
            _
            if (msg.value > _amount)
                msg.sender.send(_amount - msg.value);
        }

        function forceOwnerChange(address _newOwner)
            costs(200 ether)
        {
            owner = _newOwner;
            // just some example condition
            if (uint(owner) & 0 == 1)
                // in this case, overpaid fees will not
                // be refunded
                return;
            // otherwise, refund overpaid fees
        }}

A more specialised way in which access to function calls can be restricted will be discussed in the next example.

*************
State Machine
*************

Contracts often act as a state machine, which means
that they have certain **stages** in which they behave
differently or in which different functions can
be called. A function call often ends a stage
and transitions the contract into the next stage
(especially if the contract models **interaction**).
It is also common that some stages are automatically
reached at a certain point in **time**.

An example for this is a blind auction contract which
starts in the stage "accepting blinded bids", then
transitions to "revealing bids" which is ended by
"determine auction outcome".

.. index:: function;modifier

Function modifiers can be used in this situation
to model the states and guard against
incorrect usage of the contract.

Example
=======

In the following example,
the modifier ``atStage`` ensures that the function can
only be called at a certain stage.

Automatic timed transitions
are handled by the modifier ``timeTransitions``, which
should be used for all functions.

.. note::
    **Modifier Order Matters**.
    If atStage is combined
    with timedTransitions, make sure that you mention
    it after the latter, so that the new stage is
    taken into account.

Finally, the modifier ``transitionNext`` can be used
to automatically go to the next stage when the
function finishes.

.. note::
    **Modifier May be Skipped**.
    This only applies to Solidity before version 0.4.0:
    Since modifiers are applied by simply replacing
    code and not by using a function call,
    the code in the transitionNext modifier
    can be skipped if the function itself uses
    return. If you want to do that, make sure
    to call nextStage manually from those functions.
    Starting with version 0.4.0, modifier code
    will run even if the function explicitly returns.

::

    pragma solidity ^0.4.11;

    contract StateMachine {
        enum Stages {
            AcceptingBlindedBids,
            RevealBids,
            AnotherStage,
            AreWeDoneYet,
            Finished
        }

        // This is the current stage.
        Stages public stage = Stages.AcceptingBlindedBids;

        uint public creationTime = now;

        modifier atStage(Stages _stage) {
            require(stage == _stage);
            _;
        }

        function nextStage() internal {
            stage = Stages(uint(stage) + 1);
        }

        // Perform timed transitions. Be sure to mention
        // this modifier first, otherwise the guards
        // will not take the new stage into account.
        modifier timedTransitions() {
            if (stage == Stages.AcceptingBlindedBids &&
                        now >= creationTime + 10 days)
                nextStage();
            if (stage == Stages.RevealBids &&
                    now >= creationTime + 12 days)
                nextStage();
            // The other stages transition by transaction
            _;
        }

        // Order of the modifiers matters here!
        function bid()
            payable
            timedTransitions
            atStage(Stages.AcceptingBlindedBids)
        {
            // We will not implement that here
        }

        function reveal()
            timedTransitions
            atStage(Stages.RevealBids)
        {
        }

        // This modifier goes to the next stage
        // after the function is done.
        modifier transitionNext()
        {
            _;
            nextStage();
        }

        function g()
            timedTransitions
            atStage(Stages.AnotherStage)
            transitionNext
        {
        }

        function h()
            timedTransitions
            atStage(Stages.AreWeDoneYet)
            transitionNext
        {
        }

        function i()
            timedTransitions
            atStage(Stages.Finished)
        {
        }
    }
