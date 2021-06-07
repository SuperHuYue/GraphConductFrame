const { ipcRenderer } = require('electron');

var divRowCfm = document.getElementById('divRowCfm');
var divCfm = document.getElementById('divCfm');

divRowCfm.addEventListener('click', (e) => {
    var row = document.querySelector('#divRow').value;
    ipcRenderer.send('divShowWin:row', row);
})

divCfm.addEventListener('click', (e) => {
    var idx = document.querySelector('#divIdx').value;
    var col = document.querySelector('#divCol').value;
    ipcRenderer.send('divShowWin:idxCol', [idx, col]);
})

// form.addEventListener('submit', () => {
//     var row = document.getElementById('divRow');
//     var idx = document.getElementById('divIdx');
//     var col = document.getElementById('divCol');
//     //计算获得具体的拆分规则
//     // var sinRowPer = 100 / row;
//     var rowPer = [];
//     var singRow = Math.ceil(100 / row);
//     for (let i = 0; i < row; ++i) {
//         rowPer.push(singRow);
//     }
//     rowPer.pop();
//     rowPer.push(100 - (singRow * row));


// })
