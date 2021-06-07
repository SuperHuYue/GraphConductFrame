const { ipcRenderer } = require("electron");
const path = require('path')
const fs = require('fs');
const websocket = require('ws')
const { file } = require("electron-settings");

// const WSPARASAVEFILENAME = path.join(__dirname, 'wsPara.js');
var wsWinIdentify = 0;
ipcRenderer.on('wsWin:IDSet', (e, id) => {
    console.log("Now we get Window id is: " + id.toString());
    var obj = document.getElementsByTagName('title');
    obj[0].innerHTML = id.toString();
    wsWinIdentify = id;
});
var btn = document.getElementById("wsConnect");
btn.addEventListener('click', (e) => {
    var wsIP = document.querySelector('#wsIP').value;
    var wsPort = document.querySelector('#wsPort').value;
    console.log('wsIP:' + wsIP + 'wsPort' + wsPort);
    ipcRenderer.send('wsWin:Connect', { wsWinIdentify, wsIP, wsPort });
})



// e.preventDefault();
// var wsIP = document.querySelector('#wsIP').value;
// var wsPort = document.querySelector('#wsPort').value;
// console.log('wsIP: ' + wsIP + 'wsPort' + wsPort);
// ipcRenderer.send('wsWin:Connect', { wsWinIdentify, wsIP, wsPort });
// });

//     var para = {
//         wsIP: document.querySelector('#wsIP').value,
//         wsPort: document.querySelector('#wsPort').value,
//     }
//     fs.writeFile(WSPARASAVEFILENAME, JSON.stringify(para), (err) => {
//         if (err) console.log('写入失败');
//         console.log('写入成功 ');
//     })
//     //////////////////////////////////////////////////////////////////////
//     //连接websocket
//     //////////////////////////////////////////////////////////////////////
//     if (wsEntityOut != null) {
//         wsEntityOut.wsEntity.close();
//     }
//     wsEntityOut = new WSObj(para.wsIP, para.wsPort, "?Method=ECHO")//最后的参数与所连接的服务的通讯格式有关
//     console.log("submit over");



// ipcRenderer.on('wsWin:Init', (e) => {
//     var file_data = fs.readFileSync(WSPARASAVEFILENAME, 'utf-8');
//     file_data = JSON.parse(file_data);
//     for (key in file_data) {
//         document.getElementById(key).value = file_data[key];
//     }
// })

// var form = document.querySelector('form');
// var wsEntityOut = null;//目前暂且只做一个
// form.addEventListener('submit', (e) => {
//     e.preventDefault();//取消EventListener的默认操作 
//     var para = {
//         wsIP: document.querySelector('#wsIP').value,
//         wsPort: document.querySelector('#wsPort').value,
//     }
//     fs.writeFile(WSPARASAVEFILENAME, JSON.stringify(para), (err) => {
//         if (err) console.log('写入失败');
//         console.log('写入成功 ');
//     })
//     //////////////////////////////////////////////////////////////////////
//     //连接websocket
//     //////////////////////////////////////////////////////////////////////
//     if (wsEntityOut != null) {
//         wsEntityOut.wsEntity.close();
//     }
//     wsEntityOut = new WSObj(para.wsIP, para.wsPort, "?Method=ECHO")//最后的参数与所连接的服务的通讯格式有关
//     console.log("submit over");
// })
// class WSObj {
//     constructor(ip, port, other) {
//         this.Uri = "ws://" + ip.toString() + ":" + port.toString() + "/" + other.toString();
//         this.wsEntity = null;
//         this.reader = null;
//         this.init();
//     }
//     init() {
//         this.wsEntity = new websocket(this.Uri);
//         this.wsEntity.onopen = onOpen;
//         this.wsEntity.onclose = onClose;
//         this.wsEntity.onmessage = onMessage;
//         this.wsEntity.onerror = onError;
//         console.log("init complete..");
//     };

// };


// //必须写在外面，应为调用的内容都是websocket的this而不是我们定义的

// function onOpen(evt) {
//     console.log("open");
// };
// function onClose(evt) {
//     console.log("close");

// };
// // doSend(message) {
// //     console.log(message);
// //wsEntityOut.wsEntity.send(message);
// // };
// function onMessage(evt) {
//     var oriStr = evt.data;
//     console.log(typeof (evt.data));
//     if (typeof (evt.data) != "string") {
//         //Customer made data demo is only show pic data
//         if (wsEntityOut.reader == null) {
//             wsEntityOut.reader = new FileReader();
//             wsEntityOut.reader.addEventListener("loadend", function (e) {
//                 var buffer_header = new Uint32Array(e.target.result, 0, 10);
//                 var header_size = parseInt(buffer_header[0]);
//                 var to_size = header_size / 4 - 10;
//                 var buffer = new Uint32Array(e.target.result, 0, 10 + to_size + 4);
//                 // var version = parseInt(buffer[1]);
//                 var from_id = parseInt(buffer[2]);
//                 // var body_size = buffer[3];
//                 var to_list = [];
//                 for (var i = 0; i < to_size; ++i) {
//                     to_list[i] = buffer[i + 10];
//                 }
//                 //
//                 var row = buffer[10 + to_size];
//                 var col = buffer[11 + to_size];
//                 var deepth = buffer[12 + to_size];
//                 var pic_length = parseInt(buffer[13 + to_size]);
//                 var pic_data = new Uint8Array(e.target.result, (14 + to_size) * 4, pic_length);
//                 // var blob = new Blob([pic_data], { type: "image/jpeg" });
//                 // var url = URL.createObjectURL(blob);
//                 ipcRenderer.send("wsWin:ImgData", { data: pic_data, row: row, col: col });
//                 // document.getElementById("show_container").src = url;
//                 // document.getElementById("show_container").style.width = col + "px";
//                 // document.getElementById("show_container").style.height = row + "px";
//                 // document.getElementById("pic_container").style.width = col + "px";
//                 // document.getElementById("pic_container").style.height = row + "px";
//                 // console.log(" row = " + row + " col=" + col + " deepth=" + deepth);

//             });
//         }
//         wsEntityOut.reader.readAsArrayBuffer(new Blob([evt.data], { type: "image/jpeg" }));

//     }
//     else {
//         var str = evt.data.substr(0, 4);
//         if (str == "?Sys") {
//             if (oriStr.search("/?SysSetIDEcho") != -1) {//start by ?Sys called systype
//                 //writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data + '</span>');
//                 var pos = oriStr.search("ID=");
//                 if (pos != -1) {
//                     var id_num = 0;
//                     var id_start = pos + 3;
//                     var tmpStr = oriStr.substr(pos + 3)
//                     var id_end = tmpStr.search("&");
//                     if (id_end != -1) {
//                         id_num = tmpStr.substr(0, id_end);
//                     }
//                     else {
//                         id_num = tmpStr;
//                     }
//                     // console.log("11111")
//                     // console.log(wsEntityOut);
//                     // console.log("11111")
//                     if (wsEntityOut.wsEntity == undefined) {
//                         console.log("I do not know why");
//                     } else {
//                         wsEntityOut.wsEntity.send("?SysSetName=" + "browser" + "&ID=" + id_num.toString());
//                     }
//                     //wsEntityOut.doSend("?SysSetName=" + "browser" + "&ID=" + id_num.toString());
//                 }
//             }
//         }

//     }
// };
// function onError(evt) {
//     console.log("err ");
// };




// // var wsUri = "ws://localhost:9106/?Method=ECHO";
// // var output;
// // var id_num;
// // var websocket;
// // function init() {
// //     output = document.getElementById("connect_result");
// //     testWebSocket();
// // }
// // function connect_close() {
// //     console.log("close click...");
// //     websocket.close();
// // }
// // function start_connect() {
// //     init();
// //     console.log("connect click...");
// // }

// // function testWebSocket() {
// //     websocket = new WebSocket(wsUri);
// //     websocket.onopen = function (evt) {
// //         onOpen(evt)
// //     };
// //     websocket.onclose = function (evt) {
// //         onClose(evt)
// //     };
// //     websocket.onmessage = function (evt) {
// //         onMessage(evt)
// //     };
// //     websocket.onerror = function (evt) {
// //         onError(evt)
// //     };
// // }

// // function onOpen(evt) {
// //     writeToScreen("CONNECTED");
// //     //return true;
// // }

// // function onClose(evt) {
// //     writeToScreen("DISCONNECTED");
// // }

// // function onMessage(evt) {

// // }

// // function onError(evt) {
// //     writeToScreen('<span style="color: red;">ERROR:</span> ' + evt.data);
// // }


// // function writeToScreen(message) {
// //     var pre = document.createElement("p");
// //     pre.style.wordWrap = "break-word";
// //     pre.innerHTML = message;
// //     output.appendChild(pre);
// // }
