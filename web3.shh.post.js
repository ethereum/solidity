var chai = require('chai');
var web3 = require('../index');
var testMethod = require('./helpers/test.method.js');

var method = 'post';

var tests = [{
    args: [{
        from: '0x123123123',
        topics: ['hello_world'],
        payload: '12345',
        ttl: 100,
        workToProve: 101
    }],
    formattedArgs: [{
        from: '0x123123123',
        topics: [web3.fromAscii('hello_world')],
        payload: web3.toHex('12345'),
        ttl: web3.toHex('100'),
        workToProve: web3.toHex('101'),
        priority: '0x0'
    }],
    result: true,
    formattedResult: true,
    call: 'shh_'+ method
}, {
    args: [{
        from: '0x21312',
        topics: ['hello_world'],
        payload: '0x12345',
        ttl: 0x100,
        workToProve: 0x101,
        priority: 0x15
    }],
    formattedArgs: [{
        from: '0x21312',
        topics: [web3.fromAscii('hello_world')],
        payload: '0x12345',
        ttl: '0x100',
        workToProve: '0x101',
        priority: '0x15'
    }],
    result: true,
    formattedResult: true,
    call: 'shh_'+ method
}];

testMethod.runTests('shh', method, tests);

