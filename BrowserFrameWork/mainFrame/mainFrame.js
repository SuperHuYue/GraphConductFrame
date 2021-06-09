
const { ipcRenderer, TouchBarSlider } = require('electron');
const websocket = require('ws')

/*
id:
obj:
*/
var showObjs = {};//展示模块，用于主界面的图像展示
var arr = new Array(1);//该变量用于展示目前所拥有的所有分层的对象 
const IMGCONTAINERNAME = 'ImgContainer1';

ipcRenderer.on("wsWin:ImgData", (e, Img) => {
    Shower(Img);
});

//页面不会销毁，仅仅是关闭服务
function wsCloseAll(method = 'ws') {
    if (showObjs != null) {
        for (let key in showObjs) {
            showObjs[key].stop();//
        }
    }
}


function connect(tar, id, ip, port, method = 'ws') {
    //tar实际上是一个字典, 代表的是id号和对应的对象
    if (tar == null) {
        tar = {};
    }
    if ((id in tar)) {
        tar[id].start();
    } else {
        if (method == 'ws') {
            tar[id] = new WSObj(id, ip, port, "?Alias");
        }
        tar[id].start();
    }
}
function disConnect(tar, id) {
    if ((id in tar)) {
        tar[id].stop();
    }
}

ipcRenderer.on('divShowWin:row', (e, row) => {
    //重新拆分页面之后的操作步骤
    //1.关闭对应所有实体,停止通讯工作
    //2.检查新拆分之后的显示模块与现有的是否存在id相同的部分，如果有则保留，否则delete对应对象，并关闭对应窗口
    //3.重新分配对应并重连保留下来的模块
    wsCloseAll();
    var imgContainer = document.getElementById(IMGCONTAINERNAME);
    imgContainer.innerHTML = "";
    let show_row = row;
    arr = new Array(show_row);
    ////////////////////////////////////////
    //重新布局显示
    let sin_height = parseInt(100 / row);
    console.log("hello: " + sin_height);
    for (let i = 0; i < show_row; ++i) {
        var div = document.createElement('div');
        var img = document.createElement('img');
        img.classList.add('ShowContainer')
        img.id = 'row:' + i.toString() + 'col:0';
        div.id = 'divRow:' + i.toString();
        img.style.width = '100%';
        img.style.objectFit = 'cover';
        img.src = "../WebSocketParaSetting/img/init.jpg";
        img.style.height = '100%';
        img.onclick = (e) => {
            ipcRenderer.send('mainFrameImg:click', e.target.id);
        }
        if (i == show_row - 1) {
            // img.style.height = (100 - (sin_height * i)).toString() + '%';
            div.style.height = (100 - (sin_height * i)).toString() + '%';
        }
        else {
            div.style.height = sin_height.toString() + '%';
        }
        div.style.width = '100%';
        div.appendChild(img)
        imgContainer.appendChild(div);
    }
    ////////////////////////////////////////
    //检查窗口移除不存在对象的id
    var now_valid_id = [];
    let rows = imgContainer.childNodes;
    for (let sin = 0; sin < rows.length; sin++) {//遍历node只能这样用
        let cols = rows[sin].childNodes;
        for (let img_idx = 0; img_idx < cols.length; ++img_idx) {
            let img = cols[img_idx]
            now_valid_id.push(img.id)
        }
    }
    if (showObjs != null) {
        for (let show_idx in showObjs) {
            if (now_valid_id.find(element => element == show_idx) == undefined) {
                //销毁对象
                delete showObjs[show_idx];
            } else {
                //存在对象进行重连 
                let tar = showObjs[show_idx];
                console.log('reconnect: ', tar.id);
                connect(showObjs, tar.id, tar.ip, tar.port);
            }
        }
    }
});
//在拆分目标的同事对现存对象进行重连
ipcRenderer.on('divShowWin:idxCol', (e, idxcol) => {
    wsCloseAll();
    arr[idxcol[0]] = idxcol[1];
    ////////////////////////////////////////
    var div = document.getElementById('divRow:' + idxcol[0].toString());
    div.innerHTML = "";
    let sin_width = parseInt(100 / idxcol[1]);
    for (let i = 0; i < idxcol[1]; ++i) {
        var img = document.createElement('img');
        img.id = 'row:' + idxcol[0].toString() + 'col:' + i.toString();
        img.src = "../WebSocketParaSetting/img/init.jpg";
        img.style.height = '100%';
        img.onclick = (e) => {
            ipcRenderer.send('mainFrameImg:click', e.target.id);
        }
        if (i == idxcol[1] - 1) {
            img.style.width = (100 - (sin_width * i)).toString() + '%';
            // div.style.height = (100 - (sin_height * i)).toString() + '%';
        }
        else {
            img.style.width = sin_width.toString() + '%';
        }
        div.appendChild(img)
    }
    //检查窗口移除不存在对象的id
    var imgContainer = document.getElementById(IMGCONTAINERNAME);
    var now_valid_id = [];
    let rows = imgContainer.childNodes;
    for (let sin = 0; sin < rows.length; sin++) {//遍历node只能这样用
        let cols = rows[sin].childNodes;
        for (let img_idx = 0; img_idx < cols.length; ++img_idx) {
            let img = cols[img_idx]
            now_valid_id.push(img.id)
        }
    }
    if (showObjs != null) {
        for (let show_idx in showObjs) {
            if (now_valid_id.find(element => element == show_idx) == undefined) {
                //销毁对象
                delete showObjs[show_idx];
            } else {
                //存在对象进行重连 
                let tar = showObjs[show_idx];
                console.log('reconnect: ', tar.id);
                connect(showObjs, tar.id, tar.ip, tar.port);
            }
        }
    }
});

ipcRenderer.on('wsWin:disConnect', (e, data) => {
    var id = data.wsWinIdentify;
    console.log("close: ", id);
    disConnect(showObjs, id);
    //connect(showObjs, id, ip, port);
});

ipcRenderer.on('wsWin:Connect', (e, data) => {
    var id = data.wsWinIdentify;
    var ip = data.wsIP;
    var port = data.wsPort;
    console.log("ipcRederConnect : ", data);
    connect(showObjs, id, ip, port);
});

class baseObj {
    constructor() {
        this.method = 'unDefined';
        this.entity = null;//数据流通（通讯）使用的实体
        this.id = null;
    }
    stop() {
        //每一个都必须实现这个对象,代表停止该通讯对象工作
        throw ('You must implement the stop method...')
    }
    start() {
        //每一个对象都必须实现这个对象，代表停止该通讯对象工作
        throw ('You must implement the start method...')
    }
}

class WSObj extends baseObj {
    constructor(id, ip, port, other) {
        super();
        this.Uri = "ws://" + ip.toString() + ":" + port.toString() + "/" + other.toString() + "=" + id.toString();
        this.ip = ip;
        this.port = port;
        this.other = other;
        this.reader = null;
        this.method = 'ws';
        this.id = id;
        this.entity = null;
    }
    start() {
        this.init();
    }
    stop() {
        if (this.entity != null) {
            this.entity.close();
        }
    }
    init() {
        this.entity = new websocket(this.Uri);
        this.entity.onopen = () => {
            var sayWhat = 'open..';
            ipcRenderer.send('wsWin:connectStatus', { id: this.id, say: sayWhat });
            console.log('open..')
        };
        this.entity.onclose = (e) => {
            console.log("closed...")
            var sayWhat = 'close..';
            ipcRenderer.send('wsWin:connectStatus', { id: this.id, say: sayWhat });
            console.log('close: reason' + e.data);
        };
        this.entity.onmessage = (evt) => {
            var oriStr = evt.data;
            if (typeof (evt.data) != "string") {
                var buffer_header = new Uint8Array(evt.data, 0, 13)
                var buffer = [];//9header, 1-row,1-col,1-depth,1-piclen
                for (let i = 0; i < 4 * (9 + 4);) {
                    var tmp = buffer_header[i] | buffer_header[i + 1] << 8 | (buffer_header[i + 2]) << 16 | (buffer_header[i + 3]) << 32;
                    buffer.push(tmp);
                    i += 4;
                }
                var row = buffer[9];
                var col = buffer[10];
                var deepth = buffer[11];
                var pic_length = buffer[12];
                var pic_data = new Uint8Array(evt.data.slice(13 * 4, pic_length + 13 * 4));
                this.Shower(pic_data, this.id);

            }
            else {
                console.log(evt.data)
                var str = evt.data.substr(0, 4);
                if (str == "?Sys") {
                    if (oriStr.search("/?SysConnectInit=Got") != -1) {//start by ?Sys called systype
                        //writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data + '</span>');
                        if (this.entity == undefined) {
                            console.log("I do not know why");
                        } else {
                            // ?SysSetMethod = xxx & FromAlias=xxx & ToAlias=xx, xx, xx
                            this.entity.send("?SysSetMethod=ImgRetransmission");//这里简略的认为绝对成功，实际上是有问题的
                        }
                    }
                }
                //往后还可以添加其他参数的设定信息
                //end
            }

        }

        this.entity.onerror = (evt) => {
            console.log(evt.data);
            this.entity.close();
        };
        console.log("init complete..");
    };

    Shower(Img, id) {
        var mF = document.getElementById(id);
        // console.log(" shower:", mF);
        if (mF == null) {
            if (this.entity != null) {
                this.entity.close();
                this.entity = null;
            }
        }
        var blob = new Blob([Img], { type: "image/jpeg" });
        var url = URL.createObjectURL(blob);//这个对象使用之后需要手动进行删除，告诉浏览器该内存无需保存
        URL.revokeObjectURL(mF.src);
        mF.src = null;
        mF.src = url;
        // mF.src = ImgUrl.url.blob;
    }
};





