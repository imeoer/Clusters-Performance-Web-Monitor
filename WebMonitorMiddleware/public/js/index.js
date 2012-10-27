/*
    Project: WebMonitorMiddleware Front-end
    Modify: 2012-08-21
    Author: SWUST - YanSong,ZhangYaWen,GuoDaPeng
*/

var CPUUse = 0, MemoryUse = 0, DiskUse = 0, NodeNum = 0;
var Terminal;
var socket, chartCPUUse, chartMemoryUse, chartDiskUse;

$(document).ready(function() {
	initChart();
	initSocket();

    $('#console_input').terminal(function(command, term){
        socket.emit('str_cmd', command);
        Terminal = term;
    }, {
        greetings: '结点远程终端',
        prompt: '> '
    });
});

/*时间日期格式化*/
Date.prototype.format = function(format){
    var o = {
        "M+" : this.getMonth()+1, //month
        "d+" : this.getDate(),    //day    
        "H+" : this.getHours(),   //hour    
        "m+" : this.getMinutes(), //minute    
        "s+" : this.getSeconds(), //second    
        "q+" : Math.floor((this.getMonth()+3)/3), //quarter    
        "S" : this.getMilliseconds() //millisecond    
    }    
    if(/(y+)/.test(format))   
    format = format.replace(RegExp.$1,(this.getFullYear()+"").substr(4 - RegExp.$1.length));    
    for(var k in o)  
    if(new RegExp("("+ k +")").test(format))    
    format = format.replace(RegExp.$1, RegExp.$1.length==1 ? o[k] : ("00"+ o[k]).substr((""+ o[k]).length));    
    return format;    
}

function showTip(infoText){ //显示提示信息
    $('#tip').text(infoText);
    $('#tip').show();
    setTimeout(function(){
        $('#tip').fadeOut('normal');
    }, 3000);
}

function flashNode(nodeNum){
    var nodeElem = $('[name="' + nodeNum + '"]');
    nodeElem.css({'background-color': '#f00'});
    nodeElem.css({'color': '#fff'});
    nodeElem.fadeTo(200, 0.5, function(){
        nodeElem.fadeTo(200, 1.0);
        nodeElem.css({'background-color': '#fff'});
        nodeElem.css({'color': '#000'});
    });
}

function initSocket(){ //初始化Socket
	var strHost = 'http://' + document.domain + ':8888';
	socket = io.connect(strHost);
	socket.on('data', function(strData){
        var dataObj = eval('(' + strData + ')');
        var msgType = dataObj.msg_type;
        NodeNum = dataObj.node_num;
        if(msgType == 'data'){
            CPUUse = dataObj.cpu_use;
            MemoryUse = dataObj.memory_use;
            DiskUse = dataObj.disk_use;
        }
        else if(msgType == 'cmd'){
            var msgContent = dataObj.cmd_result;
            Terminal.echo(msgContent);
        }
        $('#title').text('集群性能WEB监控平台 - ' + NodeNum + '号结点');
	});
    socket.on('tip', function(strData){
        showTip(strData);
        var consoleValue = $('#console').val() || '事件信息日志';
        var currentValue = (new Date()).format('yyyy-MM-dd HH:mm:ss') + '：' + strData;
        $('#console').val(consoleValue + '\r\n' + currentValue);
    });
    socket.on('node', function(strData){
        var nodeNumArray = eval('(' + strData + ')');
        updateNodeInfo(nodeNumArray);
    });
    socket.on('warning', function(dataObj){
        var nodeNum = dataObj['node_num'];
        flashNode(nodeNum);
    });
}

function updateNodeInfo(nodeNumArray){ //更新结点信息
    $('#node-containers').empty();
    for(var i in nodeNumArray){
        $('#node-containers').append(
            '<div class="node-container container" name="' + nodeNumArray[i] + '">' + nodeNumArray[i] + '号结点</div>'
        );
    }
    $('.node-container').die('click').live("click",function(){ //绑定结点选择框单击事件
        var animateTime = 300;
        chartCPUUse.destroy();
        chartMemoryUse.destroy();
        chartDiskUse.destroy();
        initChart();
        $('#chart-containers').fadeTo(animateTime, 0);
        $('#title').fadeTo(animateTime, 0);
        var nodeNum = $(this).attr('name');
        socket.emit('get_node', nodeNum);
        $('#chart-containers').fadeTo(animateTime, 1);
        $('#title').fadeTo(animateTime, 1);
    });
}

function initChart(){ //初始化图表
    chartCPUUse = addLineChart('CPUUse', 'CPU占用率', 'cpu-container', '#00a2ff');
    chartMemoryUse = addLineChart('MemoryUse', '内存占用率', 'memory-container', '#ff9600');
    chartDiskUse = addLineChart('DiskUse', '磁盘IO占用率', 'disk-container', '#008e00');
}

function addLineChart(type, desText, container, color){ //创建线形图
    Highcharts.setOptions({
        global: {
            useUTC: false
        }
    });
    var chart = new Highcharts.Chart({
        chart: {
            width: 400,
            height: 200,
            renderTo: container,
            zoomType: 'x',
            spacingRight: 20,
            events: {
                load: function() {
                    var series = this.series[0];
                    setInterval(function() {
                        series.addPoint(eval(type), true, true);
                    }, 100);
                }
            }
        },
        title: {
            text: desText,
            style: {fontSize: '12px'}
        },
        subtitle: {
        },
        xAxis: {
            type: 'datetime',
            tickPixelInterval: 500,
            title: {
                text: null
            }
        },
        yAxis: {
            title: {
                text: ''
            },
            min: 0,
            max: 100,
            tickWidth : 20,
            offset: 20,
            startOnTick: true,
            showFirstLabel: false
        },
        tooltip: {
            formatter: function() {
                    return '<b>'+ this.series.name +'</b><br/>'+
                    Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) +'<br/>'+
                    '占用率：' + Highcharts.numberFormat(this.y, 2) + '%';
            }
        },
        legend: {
            enabled: false
        },
        plotOptions: {
            area: {
                fillColor: {
                    linearGradient: { x1: 0, y1: 0, x2: 0, y2: 0},
                    stops: [
                        [0, color],
                        [1, '#fff']
                    ]
                },
                lineWidth: 1,
                marker: {
                    enabled: false,
                    states: {
                        hover: {
                            enabled: true,
                            radius: 5
                        }
                    }
                },
                shadow: false,
                states: {
                    hover: {
                        lineWidth: 1
                    }
                }
            }
        },

        series: [{
            type: 'area',
            name: desText,
            pointInterval: 1,
            pointStart: (new Date()).getTime(),
            data: (function() {
                var data = [];
                for(var i = -50; i <= 0; i++) {
                    data.push(0.5);
                }
                return data;
            })()
        }]
    });
    return chart;
}
