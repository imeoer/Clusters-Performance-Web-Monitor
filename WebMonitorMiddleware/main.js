/*
	Project: WebMonitorMiddleware
	Modify: 2012-08-21
	Author: SWUST - YanSong,ZhangYaWen,GuoDaPeng
*/

var serverSocketArray = new Array();
var clientSocketArray = new Array();

var iconv = require('./modules/iconv-lite');

/*获得对象的JSON字符串表示*/
function getJSONStr(jsonObj){
	return JSON.stringify(jsonObj);
}

/*扩展函数，用于将结点socket插入至数组空白部分*/
Array.prototype.pushNode = function(socket){
	for(var i = 0; i <= this.length; i++){
		if(!this[i]){
			this[i] = socket;
			this[i].num = i + 1;
			return;
		}
	}
}

/*推送消息到Client*/
function pushMsgToClient(key, value){
	console.log(value);
	for(var i = 0; i < clientSocketArray.length; i++){
		if(clientSocketArray[i] != undefined)
			clientSocketArray[i].emit(key, value);
	}
}
/*推送数据到Client*/
function pushDataToClient(dataObj){
	var strData = getJSONStr(dataObj);
	console.log(strData);
	for(var i = 0; i < clientSocketArray.length; i++){
		if(clientSocketArray[i] != undefined){
			if(!clientSocketArray[i].get_node){
				//clientSocketArray[i].emit('data', strData);
			}
			else{
				if(clientSocketArray[i].get_node == dataObj.node_num){
					clientSocketArray[i].emit('data', strData);
				}
			}
		}
	}
}
/*推送结点号至Client，用于前端更新结点信息*/
function pushNodeArrayToClient(){
	var nodeNumArray = [];
	for(var i = 0; i < serverSocketArray.length; i++){
		if(serverSocketArray[i] != undefined)
			nodeNumArray.push(serverSocketArray[i].num);
	}
	pushMsgToClient('node', getJSONStr(nodeNumArray));
}
/*向Server请求命令*/
function requestCMDToServer(nodeNum, strCMD){
	var dataObj = {
		msg_type: 'cmd',
		str_cmd: strCMD
	}
	var strData = getJSONStr(dataObj);
	for(var i = 0; i < serverSocketArray.length; i++){
		if(serverSocketArray[i] != undefined){
			if(serverSocketArray[i].num && serverSocketArray[i].num == nodeNum){
				serverSocketArray[i].write(strData);
			}
		}
	}
}

/*Server Socket服务器*/
var net = require('net');
var serverSide = net.createServer(function(socket){
	serverSocketArray.pushNode(socket);
	var nodeNumber = socket.num;

	socket.on('connect', function(){
		var msg = '结点' + nodeNumber + '(' + socket.remoteAddress + ')加入监控';
		pushMsgToClient('tip', msg);
		socket.write(getJSONStr({
			msg_type: 'assign',
			node_num: String(nodeNumber)
		}));
		pushNodeArrayToClient();
	});
	socket.on('close', function(){
		delete serverSocketArray[nodeNumber - 1];
		pushNodeArrayToClient();
		var msg = '结点' + nodeNumber + '退出监控';
		pushMsgToClient('tip', msg);
	});
	socket.on('data', function(data){
		try{
			var dataObj = JSON.parse(data);
			var msgType = dataObj['msg_type'];
			var cpuUse = dataObj.cpu_use;
			var memoryUse = dataObj.memory_use;
			var diskUse = dataObj.disk_use;

			var dataObj = {
				msg_type: 'data',
				node_num: nodeNumber,
				cpu_use: dataObj.cpu_use,
				memory_use: dataObj.memory_use,
				disk_use: dataObj.disk_use
			};
			if(cpuUse > 90 || memoryUse > 90 || diskUse > 90){
				pushMsgToClient('warning', {'node_num': socket.num});
			}
		}
		catch(err){
			var cmdResult = iconv.decode(data, 'gbk');
			console.log(cmdResult);
			var dataObj = {
				msg_type: 'cmd',
				node_num: nodeNumber,
				cmd_result: cmdResult
			}
		}
		pushDataToClient(dataObj);
	});
});

serverSide.listen(4567, '127.0.0.1');
console.log('已启动 Server Socket服务器：4567');

/*Client Socket服务器*/
var socket = require('./modules/socket.io');
var clientSide = socket.listen(8888, {log: false});
console.log('已启动 Client Socket服务器：8888');
clientSide.sockets.on('connection', function (socket) {
	socket.get_node = 0;
	clientSocketArray.pushNode(socket);
	var clientNumber = socket.num;
	var msg = '客户端(' + socket.handshake.address.address + ')连接';
	pushMsgToClient('tip', msg);
	pushNodeArrayToClient();

	socket.on('disconnect', function (data) {
		delete clientSocketArray[clientNumber - 1];
		var msg = '客户端(' + socket.handshake.address.address + ')断开';
		pushMsgToClient('tip', msg);
	});
	socket.on('get_node', function (data) {
		socket.get_node = Number(data);
	});
	socket.on('str_cmd', function (data) {
		var strCMD = data;
		console.log(strCMD);
		requestCMDToServer(socket.get_node, strCMD);
	});
});

/*HTTP服务器*/
var express = require('./modules/express');
var path = require('path');

var static_dir = path.join(__dirname, 'public');

var app = express.createServer();
app.use(express.static(static_dir));

app.listen(80);
console.log('已启动 HTTP服务器：80');