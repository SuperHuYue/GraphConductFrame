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
ipcRenderer.on('wsWin:connectStatus', (e, data) => {
    console.log('wsWin:connstatus in there..');
    var h3 = document.createElement('h3');
    var wsWinRoot = document.getElementById('WsWinRoot');
    h3.innerText = data.say;
    wsWinRoot.appendChild(h3);
});


var btn = document.getElementById("wsConnect");
btn.addEventListener('click', (e) => {
    var wsIP = document.querySelector('#wsIP').value;
    var wsPort = document.querySelector('#wsPort').value;
    console.log('wsIP:' + wsIP + 'wsPort' + wsPort);
    ipcRenderer.send('wsWin:Connect', { wsWinIdentify, wsIP, wsPort });
})







