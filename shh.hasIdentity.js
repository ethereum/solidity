var chai = require('chai');
var web3 = require('../index');
var testMethod = require('./helpers/test.method.js');

var method = 'hasIdentity';

var tests = [{
    args: ['0x2dbab4c0612bf9caf4c195085547dc0612bf9caf4c1950855'],
    formattedArgs: ['0x2dbab4c0612bf9caf4c195085547dc0612bf9caf4c1950855'],
    result: true,
    formattedResult: true,
    call: 'shh_'+ method
}];

testMethod.runTests('shh', method, tests);

